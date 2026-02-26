#pragma once
#include "../../../backend/common/render_backend.h"
#include "../../../backend/common/window_backend.h"
#include "fail.h"
#include "ribble/window/window.h"
#include "time.h"

namespace ribble::core {

    class EngineContext {
    public:
        EngineContext();
        ~EngineContext() = default;

        [[nodiscard]] const TimeManager &time() const { return *m_timeManager; }
        TimeManager &time() { return *m_timeManager; }

        [[nodiscard]] const window::WindowContext &window() const { return *m_windowContext; }
        [[nodiscard]] window::WindowContext &window() { return *m_windowContext; }

        [[nodiscard]] const backend::RenderBackend &renderer() const { return *m_renderer; }
        [[nodiscard]] backend::RenderBackend &renderer() { return *m_renderer; }

    private:
        std::unique_ptr<TimeManager> m_timeManager;
        std::unique_ptr<window::WindowContext> m_windowContext;
        std::unique_ptr<backend::RenderBackend> m_renderer;
    };

    class Engine {
    public:
        enum class Failure {
            InitializationFailure,
            AlreadyInitialized,
            AlreadyRunning,
            AlreadyShutdown,
        };

        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        Engine(Engine &&other) noexcept = default;
        Engine &operator=(Engine &&other) noexcept = default;

        [[nodiscard]] Result<void, Failure> initialize();

        Result<void, Failure> run();
        Result<void, Failure> stop();

        [[nodiscard]] Result<void, Failure> shutdown();

        [[nodiscard]] const EngineContext &context() const;
        EngineContext &context();

    private:
        [[nodiscard]] Result<void, Failure> update();
        [[nodiscard]] Result<void, Failure> render();

        bool m_initialized{false};
        bool m_running{false};

        std::unique_ptr<EngineContext> m_context;
    };
} // namespace ribble::core
