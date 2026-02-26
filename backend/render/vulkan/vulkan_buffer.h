#pragma once

#include <vulkan/vulkan.h>
#include "backend_types.h"

namespace backend {

    class VulkanBuffer {
    public:
        VulkanBuffer() = default;
        ~VulkanBuffer();

        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer &operator=(const VulkanBuffer &) = delete;

        bool create(VkDevice device, VkPhysicalDevice physicalDevice, BufferType type, BufferUsage usage,
                    size_t sizeBytes, const void *data,
                    VkQueue transferQueue = VK_NULL_HANDLE, VkCommandPool transferCmdPool = VK_NULL_HANDLE);

        void destroy(VkDevice device);

        void upload(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool cmdPool,
                    const void *data, size_t sizeBytes);
        void upload_sub(VkDevice device, const void *data, size_t sizeBytes, size_t offsetBytes);

        [[nodiscard]] VkBuffer handle() const { return m_buffer; }
        [[nodiscard]] size_t size_bytes() const { return m_sizeBytes; }
        [[nodiscard]] BufferType type() const { return m_type; }
        [[nodiscard]] bool is_valid() const { return m_buffer != VK_NULL_HANDLE; }

    private:
        VkBuffer m_buffer{VK_NULL_HANDLE};
        VkDeviceMemory m_memory{VK_NULL_HANDLE};
        size_t m_sizeBytes{0};
        BufferType m_type{BufferType::Vertex};
        VkBufferUsageFlags m_usage{0};
    };

} // namespace backend
