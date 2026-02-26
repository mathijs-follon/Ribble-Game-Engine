#pragma once

#include <ribble/core/fail.h>
#include <vulkan/vulkan.h>
#include "../../common/render_backend.h"
#include "ribble/window/window.h"

namespace backend {

    /// Abstraction for creating Vulkan surfaces from window backends (SDL3, GLFW, etc.)
    class VulkanContext {
    public:
        virtual ~VulkanContext() = default;

        /// Create VkSurfaceKHR for the given window context.
        /// Instance must be created with extensions from get_required_extensions().
        virtual ribble::core::Result<VkSurfaceKHR, RenderBackend::Failure>
        create_surface(VkInstance instance, ribble::window::WindowContext &windowContext) = 0;

        /// Get instance extensions required for this platform.
        virtual void get_required_extensions(std::vector<const char *> &out) = 0;

        [[nodiscard]] virtual const char *backend_name() const = 0;
    };

} // namespace backend
