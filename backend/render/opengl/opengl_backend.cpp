#include "opengl_backend.h"
#include <ribble/core/logger.h>

#include "../../common/backend_types.h"

namespace ribble::backend::opengl {

    OpenGLBackend::OpenGLBackend(std::unique_ptr<OpenGLContext> context) : m_context(std::move(context)) {}

    OpenGLBackend::~OpenGLBackend() {
        if (m_initialized)
            OpenGLBackend::shutdown();
    }

    void GLAPIENTRY OpenGLBackend::gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                                     GLsizei /*length*/, const GLchar *message,
                                                     const void * /*userParam*/) {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        const char *srcStr = [source]() -> const char * {
            switch (source) {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "Window System";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "Shader Compiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "Third Party";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "Application";
                default:
                    return "Other";
            }
        }();

        const char *typeStr = [type]() -> const char * {
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:
                    return "Error";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "Deprecated";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "Undefined Behavior";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "Portability";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "Performance";
                default:
                    return "Other";
            }
        }();

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                RIBBLE_LOG_ERROR("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                RIBBLE_LOG_WARNING("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                RIBBLE_LOG_INFO("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            default:
                break;
        }
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::initialize(void *nativeWindowHandle, int width,
                                                                         int height) {
        RenderBackend::initialize(nativeWindowHandle, width, height);

        m_fbWidth = width;
        m_fbHeight = height;

        if (auto r = m_context->create(nativeWindowHandle); !r)
            return core::Fail(r.error());

        // Get OpenGL version
        int major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        RIBBLE_LOG_INFO("OpenGL {}.{} — Renderer: {} — Vendor: {}", major, minor,
                        reinterpret_cast<const char *>(glGetString(GL_RENDERER)),
                        reinterpret_cast<const char *>(glGetString(GL_VENDOR)));

#if defined(RIBBLE_DEBUG)
        if (GLAD_GL_KHR_debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(gl_debug_callback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            RIBBLE_LOG_INFO("OpenGL debug output enabled.");
        }
#endif

        m_state.reset();
        m_state.set_viewport({0, 0, width, height});
        m_state.set_clear_color(m_clearColor);
        m_state.set_depth_test(true);
        m_state.set_depth_func(DepthFunc::Less);
        m_state.set_cull_face(CullMode::Back);
        m_state.set_winding_order(WindingOrder::CounterClockwise);

        m_initialized = true;
        RIBBLE_LOG_INFO("OpenGL backend initialized ({}x{}).", width, height);
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::shutdown() {
        if (!m_initialized)
            return core::Ok();
        m_context->destroy();
        m_initialized = false;
        RIBBLE_LOG_INFO("OpenGL backend shut down.");
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::begin_frame() {
        m_state.bind_framebuffer(GL_FRAMEBUFFER, 0);
        m_state.set_viewport({0, 0, m_fbWidth, m_fbHeight});
        m_state.clear(true, true, true);
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::end_frame() {
        m_context->swap_buffers();
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::set_viewport(const Viewport &viewport) {
        m_state.set_viewport(viewport);
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::set_clear_color(const render::ColorRGBA &color) {
        m_clearColor = color;
        m_state.set_clear_color(color);
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::clear() {
        m_state.clear(true, true, true);
        return core::Ok();
    }

    core::Result<void, RenderBackend::Failure> OpenGLBackend::on_resize(int width, int height) {
        m_fbWidth = width;
        m_fbHeight = height;
        m_state.set_viewport({0, 0, width, height});
        RIBBLE_LOG_INFO("OpenGL backend resized to {}x{}.", width, height);
        return core::Ok();
    }

} // namespace ribble::backend::opengl
