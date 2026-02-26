#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace backend {

    class VulkanSync {
    public:
        VulkanSync() = default;
        ~VulkanSync();

        VulkanSync(const VulkanSync &) = delete;
        VulkanSync &operator=(const VulkanSync &) = delete;

        bool create(VkDevice device, size_t maxFramesInFlight);
        void destroy(VkDevice device);

        [[nodiscard]] VkSemaphore imageAvailable(size_t frameIndex) const;
        [[nodiscard]] VkSemaphore renderFinished(size_t frameIndex) const;
        [[nodiscard]] VkFence inFlightFence(size_t frameIndex) const;
        [[nodiscard]] size_t frameCount() const { return m_frameCount; }

    private:
        std::vector<VkSemaphore> m_imageAvailable;
        std::vector<VkSemaphore> m_renderFinished;
        std::vector<VkFence> m_inFlightFences;
        size_t m_frameCount{0};
    };

} // namespace backend
