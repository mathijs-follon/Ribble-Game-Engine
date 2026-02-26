#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace backend {

    class VulkanSwapchain {
    public:
        VulkanSwapchain() = default;
        ~VulkanSwapchain();

        VulkanSwapchain(const VulkanSwapchain &) = delete;
        VulkanSwapchain &operator=(const VulkanSwapchain &) = delete;

        struct CreateInfo {
            VkPhysicalDevice physicalDevice;
            VkDevice device;
            VkSurfaceKHR surface;
            uint32_t graphicsQueueFamily;
            uint32_t presentQueueFamily;
            int width;
            int height;
        };

        bool create(const CreateInfo &info);
        void destroy(VkDevice device);
        bool recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                      uint32_t graphicsQueueFamily, uint32_t presentQueueFamily, int width, int height);

        [[nodiscard]] VkSwapchainKHR handle() const { return m_swapchain; }
        [[nodiscard]] const std::vector<VkImage> &images() const { return m_images; }
        [[nodiscard]] const std::vector<VkImageView> &imageViews() const { return m_imageViews; }
        [[nodiscard]] VkFormat imageFormat() const { return m_imageFormat; }
        [[nodiscard]] VkExtent2D extent() const { return m_extent; }
        [[nodiscard]] size_t imageCount() const { return m_images.size(); }

    private:
        VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        VkFormat m_imageFormat{VK_FORMAT_UNDEFINED};
        VkExtent2D m_extent{};
    };

} // namespace backend
