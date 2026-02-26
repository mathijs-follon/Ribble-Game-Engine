#pragma once
#include "../../../backend/common/window_backend.h"

namespace ribble::window {

    class WindowContext {
    public:
        explicit WindowContext(backend::WindowBackendType type);
        ~WindowContext() = default;

        backend::WindowBackend *backend() { return m_windowBackend.get(); }
        [[nodiscard]] const backend::WindowBackend *backend() const { return m_windowBackend.get(); }
        std::shared_ptr<core::EventBus> event_bus() { return m_windowEventBus; }
        [[nodiscard]] bool should_close() const { return m_shouldClose; }
        [[nodiscard]] bool is_minimized() const { return m_isMinimized; }
        [[nodiscard]] int framebuffer_width() const { return m_framebufferWidth; }
        [[nodiscard]] int framebuffer_height() const { return m_framebufferHeight; }
        [[nodiscard]] float aspect_ratio() const {
            if (m_framebufferHeight == 0)
                return 1.f;
            return static_cast<float>(m_framebufferWidth) / static_cast<float>(m_framebufferHeight);
        }

    private:
        std::unique_ptr<backend::WindowBackend> m_windowBackend;
        std::shared_ptr<core::EventBus> m_windowEventBus;
        int m_framebufferWidth{0};
        int m_framebufferHeight{0};
        bool m_shouldClose{false}; // written by WindowCloseEvent handler
        bool m_isMinimized{false}; // written by WindowMinimizeEvent handler
    };
} // namespace ribble::window
