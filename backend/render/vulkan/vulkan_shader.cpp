#include "vulkan_shader.h"

namespace backend {

    VulkanShader::~VulkanShader() = default;

    bool VulkanShader::create(VkDevice device, const ShaderSource &source) {
        destroy(device);

        if (source.language != ShaderLanguage::SPIRV || source.data.empty() || source.data.size() % 4 != 0)
            return false;

        m_stage = source.stage;
        m_entryPoint = source.entryPoint.empty() ? "main" : source.entryPoint;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = source.data.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(source.data.data());

        if (vkCreateShaderModule(device, &createInfo, nullptr, &m_module) != VK_SUCCESS)
            return false;

        return true;
    }

    void VulkanShader::destroy(VkDevice device) {
        if (m_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, m_module, nullptr);
            m_module = VK_NULL_HANDLE;
        }
    }

} // namespace backend
