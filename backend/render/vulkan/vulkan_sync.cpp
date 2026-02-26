#include "vulkan_sync.h"

namespace backend {

    VulkanSync::~VulkanSync() = default;

    bool VulkanSync::create(VkDevice device, size_t maxFramesInFlight) {
        m_frameCount = maxFramesInFlight;
        m_imageAvailable.resize(maxFramesInFlight);
        m_renderFinished.resize(maxFramesInFlight);
        m_inFlightFences.resize(maxFramesInFlight);

        VkSemaphoreCreateInfo semInfo{};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < maxFramesInFlight; ++i) {
            if (vkCreateSemaphore(device, &semInfo, nullptr, &m_imageAvailable[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semInfo, nullptr, &m_renderFinished[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                destroy(device);
                return false;
            }
        }
        return true;
    }

    void VulkanSync::destroy(VkDevice device) {
        for (size_t i = 0; i < m_frameCount; ++i) {
            if (m_imageAvailable[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(device, m_imageAvailable[i], nullptr);
            if (m_renderFinished[i] != VK_NULL_HANDLE)
                vkDestroySemaphore(device, m_renderFinished[i], nullptr);
            if (m_inFlightFences[i] != VK_NULL_HANDLE)
                vkDestroyFence(device, m_inFlightFences[i], nullptr);
        }
        m_imageAvailable.clear();
        m_renderFinished.clear();
        m_inFlightFences.clear();
        m_frameCount = 0;
    }

    VkSemaphore VulkanSync::imageAvailable(size_t frameIndex) const {
        return frameIndex < m_frameCount ? m_imageAvailable[frameIndex] : VK_NULL_HANDLE;
    }

    VkSemaphore VulkanSync::renderFinished(size_t frameIndex) const {
        return frameIndex < m_frameCount ? m_renderFinished[frameIndex] : VK_NULL_HANDLE;
    }

    VkFence VulkanSync::inFlightFence(size_t frameIndex) const {
        return frameIndex < m_frameCount ? m_inFlightFences[frameIndex] : VK_NULL_HANDLE;
    }

} // namespace backend
