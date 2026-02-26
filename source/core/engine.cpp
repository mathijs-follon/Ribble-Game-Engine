#include <ribble/core/engine.h>

#include "../../backend/common/window_events.h"
#include "../../backend/render/opengl/opengl_backend.h"
#include "../../backend/render/opengl/opengl_context_sdl3.h"

using namespace ribble::core;

RIBBLE_ENUM_TO_STRING(Engine::Failure, case Engine::Failure::AlreadyInitialized : return "Already Initialized";
                      case Engine::Failure::AlreadyRunning : return "Already Running";
                      case Engine::Failure::AlreadyShutdown : return "Already Shut Down";);

namespace ribble::core {

    // EngineContext

    EngineContext::EngineContext() :
        m_timeManager{std::make_unique<TimeManager>()},
        m_windowContext{std::make_unique<window::WindowContext>(backend::WindowBackendType::SDL3)},
        m_renderer{std::make_unique<backend::opengl::OpenGLBackend>(
                std::make_unique<backend::opengl::OpenGLContextSDL3>())} {}

    // Engine

    Engine::Engine() : m_context{nullptr} {}

    Engine::~Engine() = default;

    Result<void, Engine::Failure> Engine::initialize() {
        if (m_initialized) {
            return Fail(RIBBLE_WARN(Failure::AlreadyInitialized,
                                    "You are trying to initialize the engine multiple times."));
        }
        m_context = std::make_unique<EngineContext>();

        InitializeLogger();

        if (!context().window().backend()->initialize(1400, 700, "Test Window"))
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Window initialization failed."));

        // Now initialize renderer with the window handle
        if (auto renderInitResult =
                    context().renderer().initialize(context().window().backend()->native_handle(), 1400, 700);
            !renderInitResult) {
            return Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Render backend initialization failed."));
        }

        m_initialized = true;
        RIBBLE_LOG_INFO("Engine initialized.");
        return Ok();
    }

    Result<void, Engine::Failure> Engine::run() {
        if (m_running) {
            return Fail(RIBBLE_WARN(Failure::AlreadyRunning, "You are trying to run the engine multiple times."));
        }

        m_running = true;
        RIBBLE_LOG_INFO("Engine running.");

        while (!context().window().should_close()) {
            RIBBLE_LOG_DEBUG("Frame FPS: {}, running: {}, should_close: {}", context().time().frame().fps(), m_running,
                             context().window().should_close());
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

        context().window().backend()->shutdown();
        context().renderer().shutdown();

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
