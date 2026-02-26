#pragma once
#include <chrono>
#include <functional>
#include <queue>
#include <vector>
#include <memory>

namespace ribble::core {

    using namespace std::chrono_literals;

    struct TimerReturn {
        bool scheduleAgain;
        std::chrono::milliseconds scheduleAfter;

        static TimerReturn NoRepeat() {
            return {false, 0ms};
        }

        static TimerReturn Repeat(std::chrono::milliseconds duration) {
            return {true, duration};
        }
    };

    using TimerCallback = std::function<TimerReturn()>;

    struct Timer {
        TimerCallback callback;
        std::chrono::steady_clock::time_point expiryTime;
        bool isActive{true};
    };

    struct TimerComparator {
        bool operator()(const std::shared_ptr<Timer>& a, const std::shared_ptr<Timer>& b) const {
            return a->expiryTime > b->expiryTime;
        }
    };

    class FrameTimer {
    public:
        FrameTimer();
        ~FrameTimer();

        FrameTimer(const FrameTimer&) = delete;
        FrameTimer& operator=(const FrameTimer&) = delete;
        FrameTimer(FrameTimer&& other) noexcept = default;
        FrameTimer& operator=(FrameTimer&& other) noexcept = default;

        [[nodiscard]] float delta() const { return m_deltaTime; }
        [[nodiscard]] float total() const { return m_totalTime; }
        [[nodiscard]] float fps() const { return m_fps; }
        [[nodiscard]] float target_fps() const { return m_targetFps; }

        void enable_limiting() { set_limiting(true); }
        void disable_limiting() { set_limiting(false); }
        void set_limiting(bool state);
        void set_target_fps(float fps);

        void start_frame();
        void end_frame();
    private:
        void calculate_delta_time();
        void calculate_fps();
        void limit_frame_rate() const;

        std::chrono::high_resolution_clock::time_point m_lastFrameTime;
        std::chrono::high_resolution_clock::time_point m_currentFrameTime;
        float m_deltaTime{0.0f};
        float m_totalTime{0.0f};
        float m_targetFps{60.0f};
        float m_fps{0.0f};
        float m_fpsAccumulator{0.0f};
        int m_frameCount{0};
        bool m_enableLimiting{true};
    };

    class TimeManager {
    public:
        TimeManager();
        void add_timer(std::chrono::milliseconds delay, TimerCallback cb);

        void start_frame();
        void update();
        void end_frame();
        [[nodiscard]] FrameTimer& frame();
        [[nodiscard]] const FrameTimer& frame() const;
    private:
        FrameTimer m_frameTimer;

        std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerComparator> m_timers;

    };
}
