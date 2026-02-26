#include "vulkan_pipeline.h"
#include "vulkan_conversions.h"
#include <array>
#include <cstring>

namespace backend {

    VulkanPipeline::~VulkanPipeline() = default;

    bool VulkanPipeline::create(const VulkanPipelineCreateInfo &info) {
        std::array<VkPushConstantRange, 2> pushRanges{};
        pushRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushRanges[0].offset = 0;
        pushRanges[0].size = 64; // mat4
        pushRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushRanges[1].offset = 64;
        pushRanges[1].size = 16; // vec4

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;
        layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
        layoutInfo.pPushConstantRanges = pushRanges.data();

        if (vkCreatePipelineLayout(info.device, &layoutInfo, nullptr, &m_layout) != VK_SUCCESS)
            return false;

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = info.vertexShader->module();
        vertStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = info.fragmentShader->module();
        fragStage.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

        std::array<VkVertexInputAttributeDescription, 3> attrDesc{};
        attrDesc[0].location = 0;
        attrDesc[0].binding = 0;
        attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[0].offset = 0;
        attrDesc[1].location = 1;
        attrDesc[1].binding = 0;
        attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[1].offset = 12;
        attrDesc[2].location = 2;
        attrDesc[2].binding = 0;
        attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrDesc[2].offset = 24;

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = 32;
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
        vertexInput.pVertexAttributeDescriptions = attrDesc.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(info.viewportExtent.width);
        viewport.height = static_cast<float>(info.viewportExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = info.viewportExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = to_vk_cull_mode(info.cullMode);
        rasterizer.frontFace = to_vk_front_face(info.windingOrder);
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.sampleShadingEnable = VK_FALSE;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = info.depthTest ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = info.depthWrite ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = to_vk_compare_op(info.depthFunc);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = info.blend ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = to_vk_blend_factor(info.blendSrc);
        colorBlendAttachment.dstColorBlendFactor = to_vk_blend_factor(info.blendDst);
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &colorBlendAttachment;

        std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_layout;
        pipelineInfo.renderPass = info.renderPass;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(info.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) !=
            VK_SUCCESS) {
            vkDestroyPipelineLayout(info.device, m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
            return false;
        }

        return true;
    }

    void VulkanPipeline::destroy(VkDevice device) {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }
        if (m_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
    }

} // namespace backend
