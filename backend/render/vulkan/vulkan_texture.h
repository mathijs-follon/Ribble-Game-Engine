#pragma once

#include <vulkan/vulkan.h>
#include "backend_types.h"

namespace backend {

    class VulkanTexture {
    public:
        VulkanTexture() = default;
        ~VulkanTexture();

        VulkanTexture(const VulkanTexture &) = delete;
        VulkanTexture &operator=(const VulkanTexture &) = delete;

        bool create(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool cmdPool,
                    int width, int height, TextureFormat format, const void *data);

        void destroy(VkDevice device);

        [[nodiscard]] VkImage image() const { return m_image; }
        [[nodiscard]] VkImageView imageView() const { return m_imageView; }
        [[nodiscard]] VkSampler sampler() const { return m_sampler; }
        [[nodiscard]] int width() const { return m_width; }
        [[nodiscard]] int height() const { return m_height; }
        [[nodiscard]] TextureFormat format() const { return m_format; }
        [[nodiscard]] bool is_valid() const { return m_image != VK_NULL_HANDLE; }

    private:
        VkImage m_image{VK_NULL_HANDLE};
        VkDeviceMemory m_memory{VK_NULL_HANDLE};
        VkImageView m_imageView{VK_NULL_HANDLE};
        VkSampler m_sampler{VK_NULL_HANDLE};
        int m_width{0};
        int m_height{0};
        TextureFormat m_format{TextureFormat::RGBA8};
    };

} // namespace backend
