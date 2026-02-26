#pragma once

#include <wayland-client.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/gl.h>
#include "../../common/window_events.h"
#include "opengl_context.h"

namespace backend {

    class OpenGLContextWayland final : public OpenGLContext {
    public:
        ~OpenGLContextWayland() override { destroy(); }

        ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) override {
            wl_display *display =
                static_cast<wl_display *>(windowContext.backend()->native_display_handle());
            wl_surface *surface = static_cast<wl_surface *>(windowContext.backend()->native_handle());

            if (!display || !surface) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid Wayland handles"));
            }

            m_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, display, nullptr);
            if (m_display == EGL_NO_DISPLAY) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "eglGetPlatformDisplay failed"));
            }

            EGLint major, minor;
            if (!eglInitialize(m_display, &major, &minor)) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "eglInitialize failed"));
            }

            static const EGLint configAttribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                EGL_NONE};

            EGLint numConfigs;
            if (!eglChooseConfig(m_display, configAttribs, &m_config, 1, &numConfigs) || numConfigs == 0) {
                eglTerminate(m_display);
                m_display = EGL_NO_DISPLAY;
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "eglChooseConfig failed"));
            }

            static const EGLint contextAttribs[] = {
                EGL_CONTEXT_MAJOR_VERSION, 4,
                EGL_CONTEXT_MINOR_VERSION, 6,
                EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
#ifdef RIBBLE_DEBUG
                EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
#else
                EGL_CONTEXT_FLAGS_KHR, 0,
#endif
                EGL_NONE};

            m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttribs);
            if (m_context == EGL_NO_CONTEXT) {
                eglTerminate(m_display);
                m_display = EGL_NO_DISPLAY;
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "eglCreateContext failed"));
            }

            m_surface = eglCreatePlatformWindowSurface(m_display, m_config, surface, nullptr);
            if (m_surface == EGL_NO_SURFACE) {
                eglDestroyContext(m_display, m_context);
                eglTerminate(m_display);
                m_context = EGL_NO_CONTEXT;
                m_display = EGL_NO_DISPLAY;
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                 "eglCreatePlatformWindowSurface failed"));
            }

            if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
                eglDestroySurface(m_display, m_surface);
                eglDestroyContext(m_display, m_context);
                eglTerminate(m_display);
                m_surface = EGL_NO_SURFACE;
                m_context = EGL_NO_CONTEXT;
                m_display = EGL_NO_DISPLAY;
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "eglMakeCurrent failed"));
            }

            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(eglGetProcAddress))) {
                destroy();
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                 "gladLoadGL failed — could not load OpenGL function pointers"));
            }

            eglSwapInterval(m_display, 1);

            windowContext.event_bus()->subscribe<WindowResizeEvent>(
                [this](const std::shared_ptr<ribble::core::Event> &) {
                    if (m_display != EGL_NO_DISPLAY && m_surface != EGL_NO_SURFACE) {
                        eglMakeCurrent(m_display, m_surface, m_surface, m_context);
                    }
                });

            return ribble::core::Ok();
        }

        void destroy() override {
            if (m_display != EGL_NO_DISPLAY) {
                eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (m_surface != EGL_NO_SURFACE) {
                    eglDestroySurface(m_display, m_surface);
                    m_surface = EGL_NO_SURFACE;
                }
                if (m_context != EGL_NO_CONTEXT) {
                    eglDestroyContext(m_display, m_context);
                    m_context = EGL_NO_CONTEXT;
                }
                eglTerminate(m_display);
                m_display = EGL_NO_DISPLAY;
            }
        }

        void swap_buffers() override {
            if (m_display != EGL_NO_DISPLAY && m_surface != EGL_NO_SURFACE) {
                eglSwapBuffers(m_display, m_surface);
            }
        }

        void set_swap_interval(int interval) override {
            if (m_display != EGL_NO_DISPLAY) {
                eglSwapInterval(m_display, interval);
            }
        }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (Wayland/EGL)"; }
        [[nodiscard]] bool is_valid() const override {
            return m_display != EGL_NO_DISPLAY && m_context != EGL_NO_CONTEXT;
        }

    private:
        EGLDisplay m_display = EGL_NO_DISPLAY;
        EGLContext m_context = EGL_NO_CONTEXT;
        EGLSurface m_surface = EGL_NO_SURFACE;
        EGLConfig m_config = nullptr;
    };

} // namespace backend
