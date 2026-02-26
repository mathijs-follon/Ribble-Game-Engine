#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "vulkan_context.h"

namespace backend {

    class VulkanContextGLFW final : public VulkanContext {
    public:
        ribble::core::Result<VkSurfaceKHR, RenderBackend::Failure>
        create_surface(VkInstance instance, ribble::window::WindowContext &windowContext) override;

        void get_required_extensions(std::vector<const char *> &out) override;

        [[nodiscard]] const char *backend_name() const override { return "Vulkan (GLFW)"; }
    };

} // namespace backend
