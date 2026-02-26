#pragma once
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include "../../common/window_events.h"
#include "opengl_context.h"

namespace backend {
    class OpenGLContextSDL3 final : public OpenGLContext {
    public:
        ~OpenGLContextSDL3() override { destroy(); }

        ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) override {
            m_window = static_cast<SDL_Window *>(windowContext.backend()->native_handle());

            if (!m_window) {
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid window handle provided"));
            }

            m_glContext = SDL_GL_CreateContext(m_window);
            if (!m_glContext)
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "SDL_GL_CreateContext failed: {}", SDL_GetError()));

            // Make the context current
            if (!SDL_GL_MakeCurrent(m_window, m_glContext)) {
                SDL_GL_DestroyContext(m_glContext);
                m_glContext = nullptr;
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "SDL_GL_MakeCurrent failed: {}", SDL_GetError()));
            }

            // Load OpenGL function pointers using GLAD
            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)))
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "gladLoadGL failed — could not load OpenGL function pointers"));

            {
                int w = 0, h = 0;
                if (!SDL_GetWindowSizeInPixels(m_window, &w, &h)) {
                    SDL_GetWindowSize(m_window, &w, &h);
                }
                glViewport(0, 0, w, h);
            }

            windowContext.event_bus()->subscribe<WindowResizeEvent>(
                    [this](const std::shared_ptr<ribble::core::Event> &baseEvt) {
                        if (!m_window || !m_glContext)
                            return;

                        if (SDL_GL_MakeCurrent(m_window, m_glContext) != 0)
                            return;

                        int w = 0, h = 0;
                        if (!SDL_GetWindowSizeInPixels(m_window, &w, &h)) {
                            SDL_GetWindowSize(m_window, &w, &h);
                        }

                        glViewport(0, 0, w, h);
                    });

            SDL_GL_SetSwapInterval(1);
            return ribble::core::Ok();
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
} // namespace backend
