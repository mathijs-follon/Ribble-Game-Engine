#include "vulkan_context_sdl3.h"
#include <ribble/core/logger.h>

namespace backend {

    ribble::core::Result<VkSurfaceKHR, RenderBackend::Failure>
    VulkanContextSDL3::create_surface(VkInstance instance, ribble::window::WindowContext &windowContext) {
        auto *window = static_cast<SDL_Window *>(windowContext.backend()->native_handle());
        if (!window) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid SDL window handle"));
        }

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
            return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                    "SDL_Vulkan_CreateSurface failed: {}", SDL_GetError()));
        }
        return ribble::core::Ok(surface);
    }

    void VulkanContextSDL3::get_required_extensions(std::vector<const char *> &out) {
        uint32_t count = 0;
        const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&count);
        if (extensions && count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                out.push_back(extensions[i]);
            }
        }
    }

} // namespace backend
