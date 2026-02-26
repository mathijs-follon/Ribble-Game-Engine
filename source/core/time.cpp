#include <ribble/core/time.h>
#include <thread>

namespace ribble::core {
    FrameTimer::FrameTimer()
        : m_lastFrameTime(std::chrono::high_resolution_clock::now())
        , m_currentFrameTime(m_lastFrameTime)
    {}

    FrameTimer::~FrameTimer() = default;

    void FrameTimer::set_limiting(bool state) {
        m_enableLimiting = state;
    }

    void FrameTimer::set_target_fps(float fps) {
        m_targetFps = fps;
    }

    void FrameTimer::start_frame() {
        m_currentFrameTime = std::chrono::high_resolution_clock::now();
        calculate_delta_time();
    }

    void FrameTimer::end_frame() {
        if (m_enableLimiting) {
            limit_frame_rate();
        }

        m_lastFrameTime = std::chrono::high_resolution_clock::now();

        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            m_lastFrameTime - m_currentFrameTime);
        m_deltaTime = duration.count() / 1'000'000'000.0f;
        m_totalTime += m_deltaTime;

        calculate_fps();
    }

    void FrameTimer::calculate_delta_time() {
        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            m_currentFrameTime - m_lastFrameTime);
        m_deltaTime = duration.count() / 1'000'000'000.0f;
        m_totalTime += m_deltaTime;
    }

    void FrameTimer::calculate_fps() {
        m_fpsAccumulator += m_deltaTime;
        m_frameCount++;

        if (m_fpsAccumulator >= 1.0f) {
            m_fps = static_cast<float>(m_frameCount) / m_fpsAccumulator;
            m_frameCount = 0;
            m_fpsAccumulator = 0.0f;
        }
    }

    void FrameTimer::limit_frame_rate() const {
        const auto workDoneTime = std::chrono::high_resolution_clock::now();
        const auto workDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            workDoneTime - m_currentFrameTime);

        const float targetFrameTimeNs = 1'000'000'000.0f / m_targetFps;
        const float workDoneNs = static_cast<float>(workDuration.count());

        if (workDoneNs < targetFrameTimeNs) {
            const float sleepNs = (targetFrameTimeNs - workDoneNs) - 1'000'000.0f;
            if (sleepNs > 0.0f) {
                std::this_thread::sleep_for(
                    std::chrono::nanoseconds(static_cast<long long>(sleepNs)));
            }

            const auto targetTime = m_currentFrameTime + std::chrono::nanoseconds(static_cast<long long>(targetFrameTimeNs));
            while (std::chrono::high_resolution_clock::now() < targetTime) {
                // busy wait
            }
        }
    }

    TimeManager::TimeManager() = default;

    void TimeManager::add_timer(std::chrono::milliseconds delay, TimerCallback cb) {
        const auto timer = std::make_shared<Timer>();
        timer->callback = std::move(cb);
        timer->expiryTime = std::chrono::steady_clock::now() + delay;
        timer->isActive = true;
        m_timers.push(timer);
    }

    void TimeManager::start_frame() {
        m_frameTimer.start_frame();
    }

    void TimeManager::update() {
        const auto now = std::chrono::steady_clock::now();
        std::vector<std::shared_ptr<Timer>> expiredTimers;
        std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerComparator> activeTimers;

        while (!m_timers.empty()) {
            auto timer = m_timers.top();
            m_timers.pop();

            if (!timer->isActive) {
                continue;
            }

            if (timer->expiryTime <= now) {
                expiredTimers.push_back(timer);
            } else {
                activeTimers.push(timer);
            }
        }

        m_timers = std::move(activeTimers);

        for (auto& timer : expiredTimers) {
            if (auto [scheduleAgain, scheduleAfter] = timer->callback(); scheduleAgain) {
                timer->expiryTime = std::max(timer->expiryTime + scheduleAfter, now);
                m_timers.push(timer);
            }
        }
    }

    void TimeManager::end_frame() {
        m_frameTimer.end_frame();
    }

    FrameTimer& TimeManager::frame() {
        return m_frameTimer;
    }

    const FrameTimer& TimeManager::frame() const {
        return m_frameTimer;
    }
}