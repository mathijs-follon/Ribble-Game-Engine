#pragma once
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <glad/gl.h>
#include "../../common/window_events.h"
#include "opengl_context.h"

namespace backend {

    namespace {
        void *glad_glx_loader(const char *name) {
            return reinterpret_cast<void *>(glXGetProcAddress(reinterpret_cast<const GLubyte *>(name)));
        }
    } // namespace

    class OpenGLContextX11 final : public OpenGLContext {
    public:
        ~OpenGLContextX11() override { destroy(); }

        ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) override {
            // Get the X11 window handle
            Window x11Window = reinterpret_cast<Window>(windowContext.backend()->native_handle());
            if (!x11Window) {
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Invalid window handle provided"));
            }

            // Open display connection (using the same display as the window)
            // Note: In a more sophisticated implementation, we could get the Display from the window backend
            m_display = XOpenDisplay(nullptr);
            if (!m_display) {
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "XOpenDisplay failed"));
            }

            m_window = x11Window;
            m_screen = DefaultScreen(m_display);

            // Get GLX extension functions
            int glxMajor, glxMinor;
            if (!glXQueryVersion(m_display, &glxMajor, &glxMinor)) {
                XCloseDisplay(m_display);
                m_display = nullptr;
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "GLX version query failed"));
            }

            // Choose framebuffer configuration
            static int visualAttribs[] = {GLX_X_RENDERABLE,
                                          True,
                                          GLX_DRAWABLE_TYPE,
                                          GLX_WINDOW_BIT,
                                          GLX_RENDER_TYPE,
                                          GLX_RGBA_BIT,
                                          GLX_X_VISUAL_TYPE,
                                          GLX_TRUE_COLOR,
                                          GLX_RED_SIZE,
                                          8,
                                          GLX_GREEN_SIZE,
                                          8,
                                          GLX_BLUE_SIZE,
                                          8,
                                          GLX_ALPHA_SIZE,
                                          8,
                                          GLX_DEPTH_SIZE,
                                          24,
                                          GLX_STENCIL_SIZE,
                                          8,
                                          GLX_DOUBLEBUFFER,
                                          True,
                                          GLX_CONTEXT_MAJOR_VERSION_ARB,
                                          4,
                                          GLX_CONTEXT_MINOR_VERSION_ARB,
                                          6,
                                          None};

            int fbCount = 0;
            GLXFBConfig *fbConfigs = glXChooseFBConfig(m_display, m_screen, visualAttribs, &fbCount);
            if (!fbConfigs || fbCount == 0) {
                XCloseDisplay(m_display);
                m_display = nullptr;
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "No suitable GLX framebuffer config found"));
            }

            // Use the first matching config
            m_fbConfig = fbConfigs[0];

            // Get visual info
            XVisualInfo *visualInfo = glXGetVisualFromFBConfig(m_display, m_fbConfig);
            if (!visualInfo) {
                XFree(fbConfigs);
                XCloseDisplay(m_display);
                m_display = nullptr;
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Failed to get visual info"));
            }

            // Create OpenGL context
            typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);
            glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
                    reinterpret_cast<glXCreateContextAttribsARBProc>(
                            glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXCreateContextAttribsARB")));

            if (glXCreateContextAttribsARB) {
                // Use modern context creation
                int contextAttribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB,
                                        4,
                                        GLX_CONTEXT_MINOR_VERSION_ARB,
                                        6,
                                        GLX_CONTEXT_PROFILE_MASK_ARB,
                                        GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef RIBBLE_DEBUG
                                        GLX_CONTEXT_FLAGS_ARB,
                                        GLX_CONTEXT_DEBUG_BIT_ARB,
#else
                                        GLX_CONTEXT_FLAGS_ARB,
                                        0,
#endif
                                        None};

                m_glContext = glXCreateContextAttribsARB(m_display, m_fbConfig, nullptr, True, contextAttribs);
            } else {
                // Fallback to legacy context creation
                m_glContext = glXCreateContext(m_display, visualInfo, nullptr, True);
            }

            XFree(visualInfo);
            XFree(fbConfigs);

            if (!m_glContext) {
                XCloseDisplay(m_display);
                m_display = nullptr;
                return ribble::core::Fail(
                        RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure, "Failed to create GLX context"));
            }

            // Create GLX window from the existing X11 window
            // Note: The window should ideally be created with the visual from the FB config,
            // but for compatibility we'll try to use glXCreateWindow or fall back to the window directly
            typedef GLXWindow (*glXCreateWindowProc)(Display *, GLXFBConfig, Window, const int *);
            glXCreateWindowProc glXCreateWindowFunc = reinterpret_cast<glXCreateWindowProc>(
                    glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXCreateWindow")));

            if (glXCreateWindowFunc) {
                m_glxWindow = glXCreateWindowFunc(m_display, m_fbConfig, m_window, nullptr);
                if (!m_glxWindow) {
                    // Fallback to using the window directly
                    m_glxWindow = m_window;
                }
            } else {
                // Older GLX - use window directly
                m_glxWindow = m_window;
            }

            // Make context current
            if (!glXMakeContextCurrent(m_display, m_glxWindow, m_glxWindow, m_glContext)) {
                glXDestroyContext(m_display, m_glContext);
                if (m_glxWindow != m_window) {
                    glXDestroyWindow(m_display, m_glxWindow);
                }
                XCloseDisplay(m_display);
                m_glContext = nullptr;
                m_glxWindow = 0;
                m_display = nullptr;
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "Failed to make GLX context current"));
            }

            // Load OpenGL function pointers using GLAD
            if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glad_glx_loader))) {
                glXMakeContextCurrent(m_display, None, None, nullptr);
                glXDestroyContext(m_display, m_glContext);
                if (m_glxWindow != m_window) {
                    glXDestroyWindow(m_display, m_glxWindow);
                }
                XCloseDisplay(m_display);
                m_glContext = nullptr;
                m_glxWindow = 0;
                m_display = nullptr;
                return ribble::core::Fail(RIBBLE_ERROR(RenderBackend::Failure::ContextCreationFailure,
                                                       "gladLoadGL failed — could not load OpenGL function pointers"));
            }

            // Set initial viewport
            XWindowAttributes attrs;
            if (XGetWindowAttributes(m_display, m_window, &attrs)) {
                glViewport(0, 0, attrs.width, attrs.height);
            }

            // Subscribe to resize events to update viewport
            windowContext.event_bus()->subscribe<WindowResizeEvent>(
                    [this](const std::shared_ptr<ribble::core::Event> &baseEvt) {
                        if (!m_display || !m_glContext)
                            return;

                        if (!glXMakeContextCurrent(m_display, m_glxWindow, m_glxWindow, m_glContext))
                            return;

                        XWindowAttributes attrs;
                        if (XGetWindowAttributes(m_display, m_window, &attrs)) {
                            glViewport(0, 0, attrs.width, attrs.height);
                        }
                    });

            // Set vsync (using GLX_SWAP_INTERVAL_EXT if available)
            typedef void (*glXSwapIntervalEXTProc)(Display *, GLXDrawable, int);
            glXSwapIntervalEXTProc glXSwapIntervalEXT = reinterpret_cast<glXSwapIntervalEXTProc>(
                    glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalEXT")));

            if (glXSwapIntervalEXT) {
                glXSwapIntervalEXT(m_display, m_glxWindow, 1);
            } else {
                // Try MESA extension
                typedef void (*glXSwapIntervalMESAProc)(unsigned int);
                glXSwapIntervalMESAProc glXSwapIntervalMESA = reinterpret_cast<glXSwapIntervalMESAProc>(
                        glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalMESA")));
                if (glXSwapIntervalMESA) {
                    glXSwapIntervalMESA(1);
                }
            }

            return ribble::core::Ok();
        }

        void destroy() override {
            if (m_glContext && m_display) {
                glXMakeContextCurrent(m_display, None, None, nullptr);
                glXDestroyContext(m_display, m_glContext);
                m_glContext = nullptr;
            }

            if (m_glxWindow && m_glxWindow != m_window && m_display) {
                glXDestroyWindow(m_display, m_glxWindow);
                m_glxWindow = 0;
            }

            if (m_display) {
                XCloseDisplay(m_display);
                m_display = nullptr;
            }

            m_window = 0;
            m_screen = 0;
        }

        void swap_buffers() override {
            if (m_display && m_glxWindow) {
                glXSwapBuffers(m_display, m_glxWindow);
            }
        }

        void set_swap_interval(int interval) override {
            if (!m_display || !m_glxWindow)
                return;

            typedef void (*glXSwapIntervalEXTProc)(Display *, GLXDrawable, int);
            glXSwapIntervalEXTProc glXSwapIntervalEXT = reinterpret_cast<glXSwapIntervalEXTProc>(
                    glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalEXT")));

            if (glXSwapIntervalEXT) {
                glXSwapIntervalEXT(m_display, m_glxWindow, interval);
            } else {
                // Try MESA extension
                typedef void (*glXSwapIntervalMESAProc)(unsigned int);
                glXSwapIntervalMESAProc glXSwapIntervalMESA = reinterpret_cast<glXSwapIntervalMESAProc>(
                        glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalMESA")));
                if (glXSwapIntervalMESA) {
                    glXSwapIntervalMESA(static_cast<unsigned int>(interval));
                }
            }
        }

        [[nodiscard]] const char *backend_name() const override { return "OpenGL 4.6 (X11/GLX)"; }
        [[nodiscard]] bool is_valid() const override { return m_glContext != nullptr && m_display != nullptr; }

    private:
        Display *m_display{nullptr};
        Window m_window{0};
        GLXWindow m_glxWindow{0};
        GLXContext m_glContext{nullptr};
        GLXFBConfig m_fbConfig{nullptr};
        int m_screen{0};
    };
} // namespace backend
