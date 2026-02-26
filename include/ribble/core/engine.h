#pragma once
#include "fail.h"
#include "time.h"
#include "../../../backend/common/window_backend.h"
#include "ribble/window/window.h"

namespace ribble::core {

    class EngineContext {


    public:
        EngineContext();
        ~EngineContext() = default;

        [[nodiscard]] const TimeManager& time() const { return *m_timeManager; }
        TimeManager& time() { return *m_timeManager; }

        [[nodiscard]] const window::WindowContext& window() const { return *m_windowContext; }
        [[nodiscard]] window::WindowContext& window() { return *m_windowContext; }

    private:
        std::unique_ptr<TimeManager> m_timeManager;
        std::unique_ptr<window::WindowContext> m_windowContext;
    };

    class Engine {
    public:
        enum class Failure {
            AlreadyInitialized,
            AlreadyRunning,
            AlreadyShutdown,
        };

        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        Engine(Engine&& other) noexcept = default;
        Engine& operator=(Engine&& other) noexcept = default;

        [[nodiscard]] Result<void, Failure> initialize();

        Result<void, Failure> run();
        Result<void, Failure> stop();

        [[nodiscard]] Result<void, Failure> shutdown();

        const EngineContext& context() const;
        EngineContext& context();

    private:
        [[nodiscard]] Result<void, Failure> update();
        [[nodiscard]] Result<void, Failure> render() const;

        bool m_initialized{false};
        bool m_running{false};

        std::unique_ptr<EngineContext> m_context;
    };
}
