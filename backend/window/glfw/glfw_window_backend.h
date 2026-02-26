#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <utility>
#include "../../common/window_backend.h"

namespace backend {

    class GLFWWindow : public WindowBackend {
    public:
        GLFWWindow(std::shared_ptr<ribble::core::EventBus> windowEventBus) : WindowBackend{std::move(windowEventBus)} {}
        ~GLFWWindow() override = default;

        ribble::core::Result<void, Failure> initialize(int width, int height, const char *title) override;

        ribble::core::Result<void, Failure> poll_events() override;

        ribble::core::Result<void, Failure> shutdown() override;

        [[nodiscard]] void *native_handle() const override;

    private:
        GLFWwindow *m_window = nullptr;
        bool m_shouldClose = false;
        bool m_glfwInitialized = false;
        double m_lastMouseX = 0.0;
        double m_lastMouseY = 0.0;
        bool m_firstMouseMove = true;
    };

} // namespace backend
