#include "vulkan_texture.h"
#include "vulkan_conversions.h"
#include "vulkan_memory.h"
#include <cstring>
#include <stdexcept>

namespace backend {

    namespace {
        size_t format_pixel_size(TextureFormat format) {
            switch (format) {
                case TextureFormat::R8: return 1;
                case TextureFormat::RG8: return 2;
                case TextureFormat::RGB8: return 3;
                case TextureFormat::RGBA8: return 4;
                case TextureFormat::R16F: return 2;
                case TextureFormat::RG16F: return 4;
                case TextureFormat::RGB16F: return 6;
                case TextureFormat::RGBA16F: return 8;
                case TextureFormat::R32F: return 4;
                case TextureFormat::RG32F: return 8;
                case TextureFormat::RGB32F: return 12;
                case TextureFormat::RGBA32F: return 16;
                default: return 4;
            }
        }

        void transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool cmdPool,
                                     VkImage image, VkFormat format,
                                     VkImageLayout oldLayout, VkImageLayout newLayout) {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = cmdPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer cmd = VK_NULL_HANDLE;
            vkAllocateCommandBuffers(device, &allocInfo, &cmd);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beginInfo);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags srcStage, dstStage;
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                       newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                return;
            }

            vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
        }

        void copy_buffer_to_image(VkDevice device, VkQueue queue, VkCommandPool cmdPool,
                                  VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = cmdPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer cmd = VK_NULL_HANDLE;
            vkAllocateCommandBuffers(device, &allocInfo, &cmd);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beginInfo);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, 1};
            vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);
            vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
        }
    } // namespace

    VulkanTexture::~VulkanTexture() = default;

    bool VulkanTexture::create(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue,
                               VkCommandPool cmdPool, int width, int height, TextureFormat format,
                               const void *data) {
        destroy(device);

        m_width = width;
        m_height = height;
        m_format = format;

        VkFormat vkFmt = to_vk_format(format);
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        bool ok = backend::create_image(device, physicalDevice, static_cast<uint32_t>(width),
                                        static_cast<uint32_t>(height), vkFmt, VK_IMAGE_TILING_OPTIMAL, usage,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);
        if (!ok)
            return false;

        if (data && queue != VK_NULL_HANDLE && cmdPool != VK_NULL_HANDLE) {
            size_t imageSize = width * height * format_pixel_size(format);

            VkBuffer stagingBuffer = VK_NULL_HANDLE;
            VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
            if (!backend::create_buffer(device, physicalDevice, imageSize,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        stagingBuffer, stagingMemory)) {
                backend::destroy_image(device, m_image, m_memory);
                m_image = VK_NULL_HANDLE;
                m_memory = VK_NULL_HANDLE;
                return false;
            }

            void *mapped = nullptr;
            vkMapMemory(device, stagingMemory, 0, imageSize, 0, &mapped);
            memcpy(mapped, data, imageSize);
            vkUnmapMemory(device, stagingMemory);

            transition_image_layout(device, queue, cmdPool, m_image, vkFmt,
                                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copy_buffer_to_image(device, queue, cmdPool, stagingBuffer, m_image,
                                 static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            transition_image_layout(device, queue, cmdPool, m_image, vkFmt,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            backend::destroy_buffer(device, stagingBuffer, stagingMemory);
        } else if (data) {
            backend::destroy_image(device, m_image, m_memory);
            m_image = VK_NULL_HANDLE;
            m_memory = VK_NULL_HANDLE;
            return false;
        } else if (queue != VK_NULL_HANDLE && cmdPool != VK_NULL_HANDLE) {
            transition_image_layout(device, queue, cmdPool, m_image, vkFmt,
                                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkFmt;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            backend::destroy_image(device, m_image, m_memory);
            m_image = VK_NULL_HANDLE;
            m_memory = VK_NULL_HANDLE;
            return false;
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
            vkDestroyImageView(device, m_imageView, nullptr);
            backend::destroy_image(device, m_image, m_memory);
            m_imageView = VK_NULL_HANDLE;
            m_image = VK_NULL_HANDLE;
            m_memory = VK_NULL_HANDLE;
            return false;
        }

        return true;
    }

    void VulkanTexture::destroy(VkDevice device) {
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, m_sampler, nullptr);
            m_sampler = VK_NULL_HANDLE;
        }
        if (m_imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_imageView, nullptr);
            m_imageView = VK_NULL_HANDLE;
        }
        backend::destroy_image(device, m_image, m_memory);
        m_image = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
    }

} // namespace backend
