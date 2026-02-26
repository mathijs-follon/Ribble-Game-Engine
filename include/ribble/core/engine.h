#pragma once
#include "../../../backend/common/render_backend.h"
#include "../../../backend/common/window_backend.h"
#include "fail.h"
#include "ribble/window/window.h"
#include "time.h"

namespace ribble::core {

    struct EngineContextSettings {
        struct Graphics {
#ifdef _WIN32
            backend::WindowBackendType windowBackend{backend::WindowBackendType::Win32};
#else
            backend::WindowBackendType windowBackend{backend::WindowBackendType::SDL3};
#endif
            backend::RenderBackendType renderBackend{backend::RenderBackendType::OpenGL};
        };

        struct Window {
            const char *windowTitle{"Ribble Window"};
            int windowWidth{1024};
            int windowHeight{768};

            int targetFPS{60};
            bool limitingFPS{true};
        };

        Window window;
        Graphics graphics;
    };

    class EngineContext {
    public:
        EngineContext(const EngineContextSettings &engineContextSettings);
        ~EngineContext() = default;

        [[nodiscard]] const EngineContextSettings &settings() const { return m_settings; }

        [[nodiscard]] const TimeManager &time() const { return *m_timeManager; }
        TimeManager &time() { return *m_timeManager; }

        [[nodiscard]] const window::WindowContext &window() const { return *m_windowContext; }
        [[nodiscard]] window::WindowContext &window() { return *m_windowContext; }

        [[nodiscard]] const backend::RenderBackend &renderer() const { return *m_renderer; }
        [[nodiscard]] backend::RenderBackend &renderer() { return *m_renderer; }

        [[nodiscard]] bool has_valid_backends() const {
            return m_windowContext->backend() != nullptr && m_renderer != nullptr;
        }

    private:
        EngineContextSettings m_settings;
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

        [[nodiscard]] Result<void, Failure> initialize(const EngineContextSettings &engineContextSettings);

        Result<void, Engine::Failure> create_window();
        Result<void, Engine::Failure> create_window(int width, int height, const char *title);

        Result<void, Failure> run();
        Result<void, Failure> stop();
        Result<void, Failure> shutdown();

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
