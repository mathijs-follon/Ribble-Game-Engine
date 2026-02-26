#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include "vulkan_context.h"

namespace backend {

    class VulkanContextSDL3 final : public VulkanContext {
    public:
        ribble::core::Result<VkSurfaceKHR, RenderBackend::Failure>
        create_surface(VkInstance instance, ribble::window::WindowContext &windowContext) override;

        void get_required_extensions(std::vector<const char *> &out) override;

        [[nodiscard]] const char *backend_name() const override { return "Vulkan (SDL3)"; }
    };

} // namespace backend
