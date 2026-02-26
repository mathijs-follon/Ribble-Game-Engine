#include <ribble/window/window.h>

#include "../../backend/common/window_events.h"
#include "../../backend/window/sdl3/sdl3_window_backend.h"

namespace ribble::window {

    WindowContext::WindowContext(backend::WindowBackendType type)
    : m_windowBackend{nullptr}
    , m_windowEventBus{std::make_shared<core::EventBus>()}
    {
        switch (type) {
            case backend::WindowBackendType::SDL3:
                m_windowBackend = std::make_unique<SDLWindow>(m_windowEventBus);
                break;
            case backend::WindowBackendType::GLFW:
                // TODO: Implement GLFW backend
                break;
            default:
                RIBBLE_LOG_ERROR("WindowContext: Unknown window backend type: {}", static_cast<int>(type));
                break;
        }

        m_windowEventBus->subscribe<backend::WindowCloseEvent>([this](const auto&) {
            m_shouldClose = true;
        });

        m_windowEventBus->subscribe<backend::WindowMinimizeEvent>([this](const auto& e) {
            auto& evt = static_cast<const backend::WindowMinimizeEvent&>(*e);
            m_isMinimized = evt.minimized();
        });

        m_windowEventBus->subscribe<backend::WindowResizeEvent>([this](const auto& e) {
            auto& evt = static_cast<const backend::WindowResizeEvent&>(*e);
            m_framebufferWidth = evt.width();
            m_framebufferHeight = evt.height();
        });
    }
}
