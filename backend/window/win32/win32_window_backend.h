#pragma once

#include <string>
#include <utility>

#include "../../common/window_backend.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace backend {

    class Win32WindowBackend : public WindowBackend {
    public:
        explicit Win32WindowBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus);
        ~Win32WindowBackend() override;

        ribble::core::Result<void, Failure> initialize(int width, int height, const char *title) override;
        ribble::core::Result<void, Failure> poll_events() override;
        ribble::core::Result<void, Failure> shutdown() override;

        [[nodiscard]] void *native_handle() const override { return m_hwnd; }
        /// Returns HDC for WGL context creation.
        [[nodiscard]] void *native_display_handle() const override { return m_hdc; }

    private:
        void register_window_class();
        void unregister_window_class();
        static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        HWND m_hwnd = nullptr;
        HDC m_hdc = nullptr;
        std::string m_windowClassName;
        int m_width = 0;
        int m_height = 0;
        bool m_shouldClose = false;
        double m_lastMouseX = 0.0;
        double m_lastMouseY = 0.0;
        bool m_firstMouseMove = true;
    };

} // namespace backend
