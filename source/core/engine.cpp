#include <ribble/core/engine.h>

#include "../../backend/common/backend_types.h"
#include "../../backend/common/window_events.h"
#include "../../backend/render/opengl/opengl_backend.h"
#include "../../backend/render/opengl/opengl_context_sdl3.h"
#include "opengl_context_glfw.h"
#ifdef RIBBLE_HAS_X11
#include "opengl_context_x11.h"
#endif
#ifdef RIBBLE_HAS_WAYLAND
#include "opengl_context_wayland.h"
#endif
#ifdef _WIN32
#include "opengl_context_win32.h"
#endif
#ifdef RIBBLE_HAS_VULKAN
#include "../../backend/render/vulkan/vulkan_backend.h"
#include "../../backend/render/vulkan/vulkan_context_glfw.h"
#include "../../backend/render/vulkan/vulkan_context_sdl3.h"
#endif

using namespace ribble::core;

RIBBLE_ENUM_TO_STRING(Engine::Failure,
                      case Engine::Failure::InitializationFailure : return "Initialization Failure";
                      case Engine::Failure::AlreadyInitialized : return "Already Initialized";
                      case Engine::Failure::AlreadyRunning : return "Already Running";
                      case Engine::Failure::AlreadyShutdown : return "Already Shut Down";);

namespace ribble::core {

    // EngineContext

    static std::unique_ptr<backend::RenderBackend> create_renderer(backend::WindowBackendType windowType,
                                                                   backend::RenderBackendType renderType) {
        using W = backend::WindowBackendType;
        using R = backend::RenderBackendType;

        switch (renderType) {

            case R::OpenGL: {
                std::unique_ptr<backend::OpenGLContext> ctx;
                switch (windowType) {
                    case W::SDL3:
                        ctx = std::make_unique<backend::OpenGLContextSDL3>();
                        break;
                    case W::GLFW:
                        ctx = std::make_unique<backend::OpenGLContextGLFW>();
                        break;
#ifdef RIBBLE_HAS_X11
                    case W::X11:
                        ctx = std::make_unique<backend::OpenGLContextX11>();
                        break;
#endif
#ifdef RIBBLE_HAS_WAYLAND
                    case W::Wayland:
                        ctx = std::make_unique<backend::OpenGLContextWayland>();
                        break;
#endif
#ifdef _WIN32
                    case W::Win32:
                        ctx = std::make_unique<backend::OpenGLContextWin32>();
                        break;
#endif
                    default:
                        RIBBLE_LOG_ERROR("The selected window backend is not supported on your device or by OpenGL",
                                         static_cast<int>(windowType));
                        return nullptr;
                }
                return std::make_unique<backend::OpenGLBackend>(std::move(ctx));
            }

            case R::Vulkan: {
#ifdef RIBBLE_HAS_VULKAN
                std::unique_ptr<backend::VulkanContext> vkCtx;
                switch (windowType) {
                    case W::SDL3:
                        vkCtx = std::make_unique<backend::VulkanContextSDL3>();
                        break;
                    case W::GLFW:
                        vkCtx = std::make_unique<backend::VulkanContextGLFW>();
                        break;
                    default:
                        RIBBLE_LOG_ERROR("Vulkan backend requires SDL3 or GLFW window backend");
                        return nullptr;
                }
                return std::make_unique<backend::VulkanBackend>(std::move(vkCtx));
#else
                RIBBLE_LOG_ERROR("Vulkan render backend is not available (Vulkan not found at build time).");
                return nullptr;
#endif
            }

            case R::DirectX12:
            case R::Metal:
                RIBBLE_LOG_ERROR("Render backend {} is not yet implemented.", static_cast<int>(renderType));
                return nullptr;

            default:
                RIBBLE_LOG_ERROR("Unknown render backend type {}.", static_cast<int>(renderType));
                return nullptr;
        }
    }

    EngineContext::EngineContext(const EngineContextSettings &engineContextSettings) :
        m_settings{engineContextSettings}, m_timeManager{std::make_unique<TimeManager>()},
        m_windowContext{std::make_unique<window::WindowContext>(engineContextSettings.graphics.windowBackend)},
        m_renderer{create_renderer(engineContextSettings.graphics.windowBackend,
                                   engineContextSettings.graphics.renderBackend)} {
        m_timeManager->frame().set_target_fps(static_cast<float>(engineContextSettings.window.targetFPS));
        m_timeManager->frame().set_limiting(engineContextSettings.window.limitingFPS);
    }

    // Engine

    Engine::Engine() : m_context{nullptr} {}

    Engine::~Engine() = default;

    Result<void, Engine::Failure> Engine::initialize(const EngineContextSettings &engineContextSettings) {
        if (m_initialized) {
            return Fail(RIBBLE_WARN(Failure::AlreadyInitialized,
                                    "You are trying to initialize the engine multiple times."));
        }
        m_context = std::make_unique<EngineContext>(engineContextSettings);

        if (!m_context->has_valid_backends()) {
            m_context.reset();
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure,
                                     "Window backend and/or render backend not available for the selected "
                                     "combination. Check that the requested backends are built and supported."));
        }

        InitializeLogger();

        m_initialized = true;
        RIBBLE_LOG_INFO("Engine initialized.");
        return Ok();
    }

    Result<void, Engine::Failure> Engine::create_window() {
        const auto &win = context().settings().window;
        return create_window(win.windowWidth, win.windowHeight, win.windowTitle);
    }

    Result<void, Engine::Failure> Engine::create_window(int width, int height, const char *title) {
        // Inform window backend which graphics API will be used (affects window creation flags)
        context().window().backend()->set_graphics_api(
                static_cast<backend::GraphicsAPI>(static_cast<int>(context().settings().graphics.renderBackend)));
        if (!context().window().backend()->initialize(width, height, title))
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Window initialization failed."));

        if (auto renderInitResult = context().renderer().initialize(context().window(), width, height);
            !renderInitResult) {
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Render backend initialization failed."));
        }

        RIBBLE_LOG_INFO("Window created.");

        return Ok();
    }

    Result<void, Engine::Failure> Engine::run() {
        if (m_running) {
            return Fail(RIBBLE_WARN(Failure::AlreadyRunning, "You are trying to run the engine multiple times."));
        }

        m_running = true;
        RIBBLE_LOG_INFO("Engine running.");

        while (!context().window().should_close()) {
            context().time().start_frame();

            if (auto updateResult = update(); !updateResult) {
                if (updateResult.error().is_fatal()) {
                    stop();
                    return updateResult;
                }
            }

            if (auto renderResult = render(); !renderResult) {
                if (renderResult.error().is_fatal()) {
                    stop();
                    return renderResult;
                }
            }
            context().time().end_frame();
        }

        // Window was closed, stop the engine
        m_running = false;
        return Ok();
    }

    Result<void, Engine::Failure> Engine::stop() {
        if (!m_running) {
            return Fail(
                    RIBBLE_WARN(Failure::AlreadyShutdown,
                                "Either the engine never ran or you are trying to stop the engine multiple times."));
        }

        context().window().event_bus()->dispatch(std::make_shared<backend::WindowCloseEvent>());

        RIBBLE_LOG_INFO("Stopping the engine.");
        m_running = false;
        return Ok();
    }

    Result<void, Engine::Failure> Engine::shutdown() {
        if (!m_initialized) {
            return Fail(RIBBLE_WARN(Failure::AlreadyShutdown, "Engine was never initialized or already shut down."));
        }

        // Stop the engine if it's still running
        if (m_running) {
            stop();
        }

        // Renderer must shut down first (releases GLX/OpenGL context) before the window,
        // since the GLX drawable is tied to the X11 window.
        context().renderer().shutdown();
        context().window().backend()->shutdown();

        m_initialized = false;
        RIBBLE_LOG_INFO("Engine shut down.");
        return Ok();
    }

    inline const EngineContext &Engine::context() const { return *m_context; }
    inline EngineContext &Engine::context() { return *m_context; }

    Result<void, Engine::Failure> Engine::update() {
        context().time().update();
        context().window().backend()->poll_events();
        context().window().event_bus()->process_queue();
        return Ok();
    }

    Result<void, Engine::Failure> Engine::render() {
        // Begin frame (clears the screen)
        if (auto beginResult = context().renderer().begin_frame(); !beginResult) {
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Render begin_frame failed."));
        }

        // TODO: Add actual rendering calls here

        // End frame (swaps buffers to make the window visible)
        if (auto endResult = context().renderer().end_frame(); !endResult) {
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Render end_frame failed."));
        }

        return Ok();
    }
} // namespace ribble::core
