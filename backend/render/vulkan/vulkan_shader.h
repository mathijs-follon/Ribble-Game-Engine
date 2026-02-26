#pragma once

#include <vulkan/vulkan.h>
#include "backend_types.h"
#include "shader_source.h"

namespace backend {

    class VulkanShader {
    public:
        VulkanShader() = default;
        ~VulkanShader();

        VulkanShader(const VulkanShader &) = delete;
        VulkanShader &operator=(const VulkanShader &) = delete;

        /// Create VkShaderModule from SPIR-V bytecode. Vulkan backend requires SPIR-V.
        bool create(VkDevice device, const ShaderSource &source);

        void destroy(VkDevice device);

        [[nodiscard]] VkShaderModule module() const { return m_module; }
        [[nodiscard]] ShaderStage stage() const { return m_stage; }
        [[nodiscard]] const std::string &entry_point() const { return m_entryPoint; }
        [[nodiscard]] bool is_valid() const { return m_module != VK_NULL_HANDLE; }

    private:
        VkShaderModule m_module{VK_NULL_HANDLE};
        ShaderStage m_stage{ShaderStage::Vertex};
        std::string m_entryPoint = "main";
    };

} // namespace backend
