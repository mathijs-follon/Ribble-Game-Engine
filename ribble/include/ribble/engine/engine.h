//
// Created by Mathijs Follon on 2/17/26.
//

#ifndef RIBBLE_ENGINE_H
#define RIBBLE_ENGINE_H
#include <chrono>
#include <cstdint>
#include <expected>
#include <functional>

#include "engine_config.h"
#include "ribble/error/error.h"

namespace ribble::engine {

    enum class EngineFailure : uint8_t {
        WindowCreationFailed = 0,
        RendererCreationFailed,
        LoggerInitializationFailed,
        AlreadyRunning,
        NotInitialized,
    };

    using UpdateCallback = std::function<void(float deltaTime)>;
    using RenderCallback = std::function<void()>;

    class Engine {
    public:
        Engine();
        explicit Engine(const EngineConfig& config);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        Engine(Engine&& other) noexcept;
        Engine& operator=(Engine&& other) noexcept;

        std::expected<void, error::Error> initialize();
        std::expected<void, error::Error> initialize(const EngineConfig& config);

        std::expected<void, error::Error> run();
        void stop();

        void shutdown();

        void update();
        void render() const;

        void set_update_callback(UpdateCallback callback);
        void set_render_callback(RenderCallback callback);


        [[nodiscard]] float delta_time() const { return m_deltaTime; }
        [[nodiscard]] float total_time() const { return m_totalTime; }
        [[nodiscard]] float fps() const { return m_fps; }

        [[nodiscard]] bool is_running() const { return m_isRunning; }
        [[nodiscard]] bool is_initialized() const { return m_isInitialized; }

        [[nodiscard]] const EngineConfig& config() const { return m_config; }

    private:
        void calculate_delta_time();
        void calculate_fps();
        void limit_frame_rate() const;
        void handle_window_resize(int width, int height);

        // Uses
        // TODO Engine declaration completion
        // Renderer
        // Window
        // Input stuff
        // Logger

        // SceneManager <-> Engine
        // SceneManager is updated and renders active screen by Engine.
        // SceneManager uses the render to render the active nodes
        // So maybe: Window -> Scene -> Viewport -> Nodes
        // Engine: Window, SceneEngine: SceneRenderer



        EngineConfig m_config;
        UpdateCallback m_updateCallback;
        RenderCallback m_renderCallback;

        std::chrono::high_resolution_clock::time_point m_lastFrameTime;
        std::chrono::high_resolution_clock::time_point m_currentFrameTime;
        float m_deltaTime{0.0f};
        float m_totalTime{0.0f};
        float m_fps{0.0f};
        float m_fpsAccumulator{0.0f};
        int m_frameCount{0};

        int m_lastWindowWidth{0};
        int m_lastWindowHeight{0};

        bool m_isRunning{false};
        bool m_isInitialized{false};
    };
} // ribble::engine

#endif //RIBBLE_ENGINE_H