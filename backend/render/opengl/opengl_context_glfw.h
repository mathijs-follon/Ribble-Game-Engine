#pragma once
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "opengl_context.h"

namespace ribble::backend::opengl {

    class OpenGLContextGLFW final : public OpenGLContext {
    public:
        ~OpenGLContextGLFW() override { destroy(); }

        core::Result<void, RenderBackend::Failure> create(void *nativeWindowHandle) override {
            m_window = static_cast<GLFWwindow *>(nativeWindowHandle);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(RIBBLE_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
            glfwMakeContextCurrent(m_window);

            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))
                return core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                               "gladLoadGL failed — could not load OpenGL function pointers"));

            glfwSwapInterval(1);
            return core::Ok();
        }

        void destroy() override { m_window = nullptr; }
        void swap_buffers() override { glfwSwapBuffers(m_window); }
        void set_swap_interval(int i) override { glfwSwapInterval(i); }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (GLFW)"; }
        [[nodiscard]] bool is_valid() const override { return m_window != nullptr; }

    private:
        GLFWwindow *m_window{nullptr};
    };

} // namespace ribble::backend::opengl
