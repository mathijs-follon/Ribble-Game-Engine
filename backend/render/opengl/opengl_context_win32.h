#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <glad/gl.h>

// WGL ARB extensions - required for modern OpenGL context
#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif
#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif
#ifndef WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#endif
#ifndef WGL_CONTEXT_CORE_PROFILE_BIT_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif
#ifndef WGL_CONTEXT_FLAGS_ARB
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#endif
#ifndef WGL_CONTEXT_DEBUG_BIT_ARB
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#endif

typedef BOOL(WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef BOOL(WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);

#include "../../common/window_events.h"
#include "opengl_context.h"

namespace backend {

    namespace {
        void *glad_wgl_loader(const char *name) {
            void *p = wglGetProcAddress(name);
            if (p)
                return p;
            return reinterpret_cast<void *>(GetProcAddress(GetModuleHandleA("opengl32.dll"), name));
        }
    } // namespace

    class OpenGLContextWin32 final : public OpenGLContext {
    public:
        ~OpenGLContextWin32() override { destroy(); }

        ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) override {
            HDC hdc = static_cast<HDC>(windowContext.backend()->native_display_handle());
            HWND hwnd = static_cast<HWND>(windowContext.backend()->native_handle());

            if (!hdc || !hwnd) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid window/DC handle provided"));
            }

            m_hdc = hdc;
            m_hwnd = hwnd;

            // Create temporary context to load WGL extensions
            PIXELFORMATDESCRIPTOR pfd = {};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;

            int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
            if (!pixelFormat) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "ChoosePixelFormat failed"));
            }

            if (!SetPixelFormat(m_hdc, pixelFormat, &pfd)) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "SetPixelFormat failed"));
            }

            HGLRC tempContext = wglCreateContext(m_hdc);
            if (!tempContext) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "wglCreateContext failed"));
            }

            if (!wglMakeCurrent(m_hdc, tempContext)) {
                wglDeleteContext(tempContext);
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "wglMakeCurrent failed"));
            }

            // Load WGL ARB functions
            auto wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(
                wglGetProcAddress("wglChoosePixelFormatARB"));
            auto wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
                wglGetProcAddress("wglCreateContextAttribsARB"));

            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);

            if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
                // Fall back to legacy context
                m_glContext = wglCreateContext(m_hdc);
                if (!m_glContext) {
                    return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "wglCreateContext (legacy) failed"));
                }
            } else {
                // Use modern context - we already set a pixel format, we need to create a new window or use the existing
                // one. Since SetPixelFormat can only be called once, and we already called it with a basic format,
                // we need to use wglChoosePixelFormatARB and SetPixelFormat on a fresh DC. But our window already
                // has a pixel format set. So we cannot change it. The basic format we set should work for OpenGL 4.6
                // on modern drivers. Let's try creating the attribs context with the current pixel format.

                int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 4, WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef RIBBLE_DEBUG
                                WGL_CONTEXT_FLAGS_ARB,
                                WGL_CONTEXT_DEBUG_BIT_ARB,
#else
                                WGL_CONTEXT_FLAGS_ARB,
                                0,
#endif
                                0};

                m_glContext = wglCreateContextAttribsARB(m_hdc, nullptr, attribs);
                if (!m_glContext) {
                    // Fallback to legacy
                    m_glContext = wglCreateContext(m_hdc);
                }
            }

            if (!m_glContext) {
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Failed to create WGL context"));
            }

            if (!wglMakeCurrent(m_hdc, m_glContext)) {
                wglDeleteContext(m_glContext);
                m_glContext = nullptr;
                return ribble::core::Fail(
                    RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "wglMakeCurrent failed"));
            }

            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glad_wgl_loader))) {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(m_glContext);
                m_glContext = nullptr;
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "gladLoadGL failed - could not load OpenGL function pointers"));
            }

            RECT rect;
            if (GetClientRect(m_hwnd, &rect)) {
                int w = rect.right - rect.left;
                int h = rect.bottom - rect.top;
                if (w > 0 && h > 0)
                    glViewport(0, 0, w, h);
            }

            windowContext.event_bus()->subscribe<WindowResizeEvent>(
                [this](const std::shared_ptr<ribble::core::Event> &) {
                    if (!m_hdc || !m_glContext)
                        return;
                    wglMakeCurrent(m_hdc, m_glContext);
                    RECT rect;
                    if (GetClientRect(m_hwnd, &rect)) {
                        int w = rect.right - rect.left;
                        int h = rect.bottom - rect.top;
                        if (w > 0 && h > 0)
                            glViewport(0, 0, w, h);
                    }
                });

            // VSync via WGL_EXT_swap_control
            auto wglSwapIntervalEXT =
                reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
            if (wglSwapIntervalEXT)
                wglSwapIntervalEXT(1);

            return ribble::core::Ok();
        }

        void destroy() override {
            if (m_glContext && m_hdc) {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(m_glContext);
                m_glContext = nullptr;
            }
            m_hdc = nullptr;
            m_hwnd = nullptr;
        }

        void swap_buffers() override {
            if (m_hdc)
                SwapBuffers(m_hdc);
        }

        void set_swap_interval(int interval) override {
            auto wglSwapIntervalEXT =
                reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
            if (wglSwapIntervalEXT)
                wglSwapIntervalEXT(interval);
        }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (Win32/WGL)"; }
        [[nodiscard]] bool is_valid() const override { return m_glContext != nullptr && m_hdc != nullptr; }

    private:
        HDC m_hdc{nullptr};
        HWND m_hwnd{nullptr};
        HGLRC m_glContext{nullptr};
    };

} // namespace backend
