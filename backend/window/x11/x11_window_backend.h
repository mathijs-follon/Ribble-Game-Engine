#pragma once

#include <X11/Xlib.h>
#include <utility>
#include "../../common/window_backend.h"

namespace backend {

    class X11Window : public WindowBackend {
    public:
        X11Window(std::shared_ptr<ribble::core::EventBus> windowEventBus) : WindowBackend{std::move(windowEventBus)} {}
        ~X11Window() override = default;

        ribble::core::Result<void, WindowBackend::Failure> initialize(int width, int height,
                                                                      const char *title) override;

        ribble::core::Result<void, WindowBackend::Failure> poll_events() override;

        ribble::core::Result<void, WindowBackend::Failure> shutdown() override;

        [[nodiscard]] void *native_handle() const override;

    private:
        Display *m_display = nullptr;
        Window m_window = 0;
        int m_screen = 0;
        bool m_shouldClose = false;
        bool m_x11Initialized = false;
        double m_lastMouseX = 0.0;
        double m_lastMouseY = 0.0;
        bool m_firstMouseMove = true;
    };

} // namespace backend
