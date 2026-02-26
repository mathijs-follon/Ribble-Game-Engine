#include <ribble/core/engine.h>

using namespace ribble::core;

RIBBLE_ENUM_TO_STRING(Engine::Failure, case Engine::Failure::AlreadyInitialized : return "Already Initialized";
                      case Engine::Failure::AlreadyRunning : return "Already Running";
                      case Engine::Failure::AlreadyShutdown : return "Already Shut Down";);

namespace ribble::core {

    // EngineContext

    EngineContext::EngineContext() :
        m_timeManager{std::make_unique<TimeManager>()},
        m_windowContext{std::make_unique<window::WindowContext>(backend::WindowBackendType::SDL3)} {}

    // Engine

    Engine::Engine() : m_context{nullptr} {}

    Engine::~Engine() {}

    Result<void, Engine::Failure> Engine::initialize() {
        if (m_initialized) {
            return Fail(RIBBLE_WARN(Failure::AlreadyInitialized,
                                    "You are trying to initialize the engine multiple times."));
        }

        InitializeLogger();

        m_context = std::make_unique<EngineContext>();
        RIBBLE_LOG_INFO("Engine initialized.");
        return Ok();
    }

    Result<void, Engine::Failure> Engine::run() {
        if (m_running) {
            return Fail(RIBBLE_WARN(Failure::AlreadyRunning, "You are trying to run the engine multiple times."));
        }

        m_running = true;
        RIBBLE_LOG_INFO("Engine running.");
        while (m_running && !context().window().should_close()) {
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
            RIBBLE_LOG_INFO("FPS: {}", context().time().frame().fps());
            context().time().end_frame();
        }

        return Ok();
    }

    Result<void, Engine::Failure> Engine::stop() {
        if (!m_running) {
            return Fail(
                    RIBBLE_WARN(Failure::AlreadyShutdown,
                                "Either the engine never ran or you are trying to stop the engine multiple times."));
        }
        RIBBLE_LOG_INFO("Stopping the engine.");
        m_running = false;
        return Ok();
    }

    Result<void, Engine::Failure> Engine::shutdown() {
        if (m_running) {
            return Fail(RIBBLE_WARN(
                    Failure::AlreadyShutdown,
                    "Either the engine never ran or you are trying to shut the engine down multiple times."));
        }

        RIBBLE_LOG_INFO("Engine shut down.");
        return Ok();
    }

    inline const EngineContext &Engine::context() const { return *m_context; }
    inline EngineContext &Engine::context() { return *m_context; }

    Result<void, Engine::Failure> Engine::update() {
        context().time().update();
        // context().input().update();
        return Ok();
    }

    Result<void, Engine::Failure> Engine::render() const { return Ok(); }
} // namespace ribble::core
