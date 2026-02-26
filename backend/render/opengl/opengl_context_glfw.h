#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "../../common/window_events.h"
#include "opengl_context.h"

namespace backend {

    class OpenGLContextGLFW final : public OpenGLContext {
    public:
        ~OpenGLContextGLFW() override { destroy(); }

        ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) override {
            m_window = static_cast<GLFWwindow *>(windowContext.backend()->native_handle());

            if (!m_window) {
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid window handle provided"));
            }

            // Make the OpenGL context current (window was already created with OpenGL hints by GLFWWindow)
            glfwMakeContextCurrent(m_window);

            // Load OpenGL function pointers using GLAD
            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "gladLoadGL failed — could not load OpenGL function pointers"));

            // Set initial viewport
            int w = 0, h = 0;
            glfwGetFramebufferSize(m_window, &w, &h);
            if (w > 0 && h > 0) {
                glViewport(0, 0, w, h);
            }

            // Subscribe to resize events to update viewport
            windowContext.event_bus()->subscribe<WindowResizeEvent>(
                    [this](const std::shared_ptr<ribble::core::Event> &baseEvt) {
                        if (!m_window)
                            return;
                        glfwMakeContextCurrent(m_window);
                        int w = 0, h = 0;
                        glfwGetFramebufferSize(m_window, &w, &h);
                        if (w > 0 && h > 0) {
                            glViewport(0, 0, w, h);
                        }
                    });

            // Set vsync
            glfwSwapInterval(1);
            return ribble::core::Ok();
        }

        void destroy() override { m_window = nullptr; }
        void swap_buffers() override {
            if (m_window) {
                glfwSwapBuffers(m_window);
            }
        }
        void set_swap_interval(int i) override { glfwSwapInterval(i); }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (GLFW)"; }
        [[nodiscard]] bool is_valid() const override { return m_window != nullptr; }

    private:
        GLFWwindow *m_window{nullptr};
    };

} // namespace backend
