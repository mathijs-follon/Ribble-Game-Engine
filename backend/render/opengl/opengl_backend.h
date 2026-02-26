#pragma once
#include <memory>
#include "../../common/render_backend.h"
#include "opengl_context.h"
#include "opengl_state.h"
#include "ribble/render/color.h"

namespace ribble::backend::opengl {

    class OpenGLBackend : public RenderBackend {
    public:
        explicit OpenGLBackend(std::unique_ptr<OpenGLContext> context);
        ~OpenGLBackend() override;

        OpenGLBackend(const OpenGLBackend &) = delete;
        OpenGLBackend &operator=(const OpenGLBackend &) = delete;


        core::Result<void, Failure> initialize(void *nativeWindowHandle, int width, int height) override;
        core::Result<void, Failure> shutdown() override;

        core::Result<void, Failure> begin_frame() override;
        core::Result<void, Failure> end_frame() override;

        core::Result<void, Failure> set_viewport(const Viewport &viewport) override;
        core::Result<void, Failure> set_clear_color(const render::ColorRGBA &color) override;
        core::Result<void, Failure> clear() override;

        core::Result<void, Failure> on_resize(int width, int height) override;

        [[nodiscard]] const char *backend_name() const override { return m_context->backend_name(); }

        [[nodiscard]] OpenGLState &state() { return m_state; }
        [[nodiscard]] const OpenGLState &state() const { return m_state; }
        [[nodiscard]] int framebuffer_width() const { return m_fbWidth; }
        [[nodiscard]] int framebuffer_height() const { return m_fbHeight; }

    private:
        std::unique_ptr<OpenGLContext> m_context;
        OpenGLState m_state;
        render::ColorRGBA m_clearColor{0.1f, 0.1f, 0.1f, 1.f};
        int m_fbWidth{0};
        int m_fbHeight{0};

        static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                                 const GLchar *message, const void *userParam);
    };

} // namespace ribble::backend::opengl
