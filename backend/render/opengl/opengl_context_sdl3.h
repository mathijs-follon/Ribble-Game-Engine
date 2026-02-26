#pragma once
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include "opengl_context.h"

namespace ribble::backend::opengl {

    class OpenGLContextSDL3 final : public OpenGLContext {
    public:
        ~OpenGLContextSDL3() override { destroy(); }

        core::Result<void, RenderBackend::Failure> create(void *nativeWindowHandle) override {
            m_window = static_cast<SDL_Window *>(nativeWindowHandle);

            if (!m_window) {
                return core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid window handle provided"));
            }

            // OpenGL attributes should already be set before window creation
            // Just create the context from the existing window
            m_glContext = SDL_GL_CreateContext(m_window);
            if (!m_glContext)
                return core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                               "SDL_GL_CreateContext failed: {}", SDL_GetError()));

            // Make the context current
            if (!SDL_GL_MakeCurrent(m_window, m_glContext)) {
                SDL_GL_DestroyContext(m_glContext);
                m_glContext = nullptr;
                return core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                               "SDL_GL_MakeCurrent failed: {}", SDL_GetError()));
            }

            // Load OpenGL function pointers using GLAD
            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)))
                return core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                               "gladLoadGL failed — could not load OpenGL function pointers"));

            SDL_GL_SetSwapInterval(1);
            return core::Ok();
        }

        void destroy() override {
            if (m_glContext && m_window) {
                SDL_GL_MakeCurrent(m_window, nullptr);
                SDL_GL_DestroyContext(m_glContext);
                m_glContext = nullptr;
            }
            m_window = nullptr;
        }

        void swap_buffers() override {
            if (m_window) {
                SDL_GL_SwapWindow(m_window);
            }
        }
        void set_swap_interval(int i) override { SDL_GL_SetSwapInterval(i); }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (SDL3)"; }
        [[nodiscard]] bool is_valid() const override { return m_glContext != nullptr; }

    private:
        SDL_Window *m_window{nullptr};
        SDL_GLContext m_glContext{nullptr};
    };

} // namespace ribble::backend::opengl
