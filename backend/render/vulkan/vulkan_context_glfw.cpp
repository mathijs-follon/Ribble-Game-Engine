#include "vulkan_context_glfw.h"

namespace backend {

    ribble::core::Result<VkSurfaceKHR, RenderBackend::Failure>
    VulkanContextGLFW::create_surface(VkInstance instance, ribble::window::WindowContext &windowContext) {
        auto *window = static_cast<GLFWwindow *>(windowContext.backend()->native_handle());
        if (!window) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid GLFW window handle"));
        }

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
        if (result != VK_SUCCESS) {
            return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                    "glfwCreateWindowSurface failed: {}", static_cast<int>(result)));
        }
        return ribble::core::Ok(surface);
    }

    void VulkanContextGLFW::get_required_extensions(std::vector<const char *> &out) {
        uint32_t count = 0;
        const char **extensions = glfwGetRequiredInstanceExtensions(&count);
        if (extensions && count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                out.push_back(extensions[i]);
            }
        }
    }

} // namespace backend
