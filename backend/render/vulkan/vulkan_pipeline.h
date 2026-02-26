#pragma once

#include <vulkan/vulkan.h>
#include "backend_types.h"
#include "vulkan_shader.h"

namespace backend {

    struct VulkanPipelineCreateInfo {
        VkDevice device;
        VkRenderPass renderPass;
        VkFormat colorAttachmentFormat;
        const VulkanShader *vertexShader;
        const VulkanShader *fragmentShader;
        VkExtent2D viewportExtent;
        bool depthTest{true};
        bool depthWrite{true};
        DepthFunc depthFunc{DepthFunc::Less};
        bool blend{false};
        BlendFactor blendSrc{BlendFactor::SrcAlpha};
        BlendFactor blendDst{BlendFactor::OneMinusSrcAlpha};
        CullMode cullMode{CullMode::Back};
        WindingOrder windingOrder{WindingOrder::CounterClockwise};
    };

    class VulkanPipeline {
    public:
        VulkanPipeline() = default;
        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline &) = delete;
        VulkanPipeline &operator=(const VulkanPipeline &) = delete;

        bool create(const VulkanPipelineCreateInfo &info);
        void destroy(VkDevice device);

        [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
        [[nodiscard]] VkPipelineLayout layout() const { return m_layout; }
        [[nodiscard]] bool is_valid() const { return m_pipeline != VK_NULL_HANDLE; }

    private:
        VkPipeline m_pipeline{VK_NULL_HANDLE};
        VkPipelineLayout m_layout{VK_NULL_HANDLE};
    };

} // namespace backend
