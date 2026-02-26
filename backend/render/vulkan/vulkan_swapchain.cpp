#include "vulkan_swapchain.h"
#include <algorithm>
#include <limits>

namespace backend {

    namespace {
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
            for (const auto &f : formats) {
                if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return f;
            }
            return formats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &modes) {
            for (const auto &m : modes) {
                if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                    return m;
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &caps, int width, int height) {
            if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return caps.currentExtent;
            }
            VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
            extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
            return extent;
        }
    } // namespace

    VulkanSwapchain::~VulkanSwapchain() = default;

    bool VulkanSwapchain::create(const CreateInfo &info) {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.physicalDevice, info.surface, &caps);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(info.physicalDevice, info.surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(info.physicalDevice, info.surface, &formatCount, formats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(info.physicalDevice, info.surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(info.physicalDevice, info.surface, &presentModeCount,
                                                  presentModes.data());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
        m_extent = chooseSwapExtent(caps, info.width, info.height);

        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
            imageCount = caps.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = info.surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = m_extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {info.graphicsQueueFamily, info.presentQueueFamily};
        if (info.graphicsQueueFamily != info.presentQueueFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = caps.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(info.device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
            return false;
        }

        m_device = info.device;
        m_imageFormat = surfaceFormat.format;

        vkGetSwapchainImagesKHR(info.device, m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(info.device, m_swapchain, &imageCount, m_images.data());

        m_imageViews.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_imageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(info.device, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
                destroy(info.device);
                return false;
            }
        }
        return true;
    }

    void VulkanSwapchain::destroy(VkDevice device) {
        for (auto view : m_imageViews) {
            if (view != VK_NULL_HANDLE)
                vkDestroyImageView(device, view, nullptr);
        }
        m_imageViews.clear();
        m_images.clear();

        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
        m_device = VK_NULL_HANDLE;
    }

    bool VulkanSwapchain::recreate(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                   uint32_t graphicsQueueFamily, uint32_t presentQueueFamily, int width,
                                   int height) {
        destroy(device);
        CreateInfo info{};
        info.physicalDevice = physicalDevice;
        info.device = device;
        info.surface = surface;
        info.graphicsQueueFamily = graphicsQueueFamily;
        info.presentQueueFamily = presentQueueFamily;
        info.width = width;
        info.height = height;
        return create(info);
    }

} // namespace backend
