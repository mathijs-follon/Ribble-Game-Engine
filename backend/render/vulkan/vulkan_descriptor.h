#pragma once

#include <vulkan/vulkan.h>

namespace backend {

    // Descriptor sets for textures/UBOs - placeholder for future use
    // Currently using push constants for uniforms

    class VulkanDescriptor {
    public:
        VulkanDescriptor() = default;
        ~VulkanDescriptor() = default;
        // Reserved for texture/UBO binding
    };

} // namespace backend
