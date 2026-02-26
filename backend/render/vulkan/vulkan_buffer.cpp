#include "vulkan_buffer.h"
#include "vulkan_conversions.h"
#include "vulkan_memory.h"
#include <cstring>

namespace backend {

    namespace {
        VkBufferUsageFlags to_vk_usage(BufferType type, BufferUsage usage) {
            VkBufferUsageFlags u = 0;
            switch (type) {
                case BufferType::Vertex:
                    u = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                    break;
                case BufferType::Index:
                    u = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                    break;
                case BufferType::Uniform:
                    u = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                    break;
                case BufferType::ShaderStorage:
                    u = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                    break;
                default:
                    u = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            return u;
        }

        VkMemoryPropertyFlags to_mem_flags(BufferUsage usage) {
            switch (usage) {
                case BufferUsage::Dynamic:
                case BufferUsage::Stream:
                    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                case BufferUsage::Static:
                default:
                    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            }
        }
    } // namespace

    VulkanBuffer::~VulkanBuffer() = default;

    bool VulkanBuffer::create(VkDevice device, VkPhysicalDevice physicalDevice, BufferType type,
                              BufferUsage usage, size_t sizeBytes, const void *data,
                              VkQueue transferQueue, VkCommandPool transferCmdPool) {
        destroy(device);

        m_type = type;
        m_sizeBytes = sizeBytes;
        m_usage = to_vk_usage(type, usage);

        VkMemoryPropertyFlags memProps = to_mem_flags(usage);
        if (data && usage == BufferUsage::Static) {
            memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        bool ok = backend::create_buffer(device, physicalDevice, sizeBytes, m_usage, memProps, m_buffer, m_memory);
        if (!ok)
            return false;

        if (data) {
            if (memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                void *mapped = nullptr;
                if (vkMapMemory(device, m_memory, 0, sizeBytes, 0, &mapped) == VK_SUCCESS) {
                    memcpy(mapped, data, sizeBytes);
                    vkUnmapMemory(device, m_memory);
                }
            } else if (transferQueue != VK_NULL_HANDLE && transferCmdPool != VK_NULL_HANDLE) {
                upload(device, physicalDevice, transferQueue, transferCmdPool, data, sizeBytes);
            }
        }
        return true;
    }

    void VulkanBuffer::destroy(VkDevice device) {
        backend::destroy_buffer(device, m_buffer, m_memory);
        m_buffer = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
        m_sizeBytes = 0;
    }

    void VulkanBuffer::upload(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue,
                              VkCommandPool cmdPool, const void *data, size_t sizeBytes) {
        if (!data || sizeBytes == 0 || queue == VK_NULL_HANDLE || cmdPool == VK_NULL_HANDLE)
            return;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        if (!backend::create_buffer(device, physicalDevice, sizeBytes,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    stagingBuffer, stagingMemory)) {
            return;
        }

        void *mapped = nullptr;
        if (vkMapMemory(device, stagingMemory, 0, sizeBytes, 0, &mapped) != VK_SUCCESS) {
            backend::destroy_buffer(device, stagingBuffer, stagingMemory);
            return;
        }
        memcpy(mapped, data, sizeBytes);
        vkUnmapMemory(device, stagingMemory);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = cmdPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
            backend::destroy_buffer(device, stagingBuffer, stagingMemory);
            return;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = sizeBytes;
        vkCmdCopyBuffer(cmd, stagingBuffer, m_buffer, 1, &copyRegion);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
        backend::destroy_buffer(device, stagingBuffer, stagingMemory);
    }

    void VulkanBuffer::upload_sub(VkDevice device, const void *data, size_t sizeBytes, size_t offsetBytes) {
        if (!data || sizeBytes == 0 || m_memory == VK_NULL_HANDLE || offsetBytes + sizeBytes > m_sizeBytes)
            return;

        void *mapped = nullptr;
        if (vkMapMemory(device, m_memory, offsetBytes, sizeBytes, 0, &mapped) == VK_SUCCESS) {
            memcpy(mapped, data, sizeBytes);
            vkUnmapMemory(device, m_memory);
        }
    }

} // namespace backend
