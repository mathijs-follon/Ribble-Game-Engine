// Include our event types before any lib that might define conflicting macros
#include "../../common/window_events.h"

#include <ribble/core/logger.h>

#include "win32_window_backend.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>

using namespace ribble::core;

namespace backend {

    namespace {

        static KeyboardKey vk_to_ribble(int vk) {
            switch (vk) {
                case VK_SPACE:
                    return KeyboardKey::Space;
                case VK_OEM_7:
                    return KeyboardKey::Apostrophe;
                case VK_OEM_COMMA:
                    return KeyboardKey::Comma;
                case VK_OEM_MINUS:
                    return KeyboardKey::Minus;
                case VK_OEM_PERIOD:
                    return KeyboardKey::Period;
                case VK_OEM_2:
                    return KeyboardKey::Slash;
                case '0':
                    return KeyboardKey::Num0;
                case '1':
                    return KeyboardKey::Num1;
                case '2':
                    return KeyboardKey::Num2;
                case '3':
                    return KeyboardKey::Num3;
                case '4':
                    return KeyboardKey::Num4;
                case '5':
                    return KeyboardKey::Num5;
                case '6':
                    return KeyboardKey::Num6;
                case '7':
                    return KeyboardKey::Num7;
                case '8':
                    return KeyboardKey::Num8;
                case '9':
                    return KeyboardKey::Num9;
                case VK_OEM_1:
                    return KeyboardKey::Semicolon;
                case VK_OEM_PLUS:
                    return KeyboardKey::Equal;
                case 'A':
                    return KeyboardKey::A;
                case 'B':
                    return KeyboardKey::B;
                case 'C':
                    return KeyboardKey::C;
                case 'D':
                    return KeyboardKey::D;
                case 'E':
                    return KeyboardKey::E;
                case 'F':
                    return KeyboardKey::F;
                case 'G':
                    return KeyboardKey::G;
                case 'H':
                    return KeyboardKey::H;
                case 'I':
                    return KeyboardKey::I;
                case 'J':
                    return KeyboardKey::J;
                case 'K':
                    return KeyboardKey::K;
                case 'L':
                    return KeyboardKey::L;
                case 'M':
                    return KeyboardKey::M;
                case 'N':
                    return KeyboardKey::N;
                case 'O':
                    return KeyboardKey::O;
                case 'P':
                    return KeyboardKey::P;
                case 'Q':
                    return KeyboardKey::Q;
                case 'R':
                    return KeyboardKey::R;
                case 'S':
                    return KeyboardKey::S;
                case 'T':
                    return KeyboardKey::T;
                case 'U':
                    return KeyboardKey::U;
                case 'V':
                    return KeyboardKey::V;
                case 'W':
                    return KeyboardKey::W;
                case 'X':
                    return KeyboardKey::X;
                case 'Y':
                    return KeyboardKey::Y;
                case 'Z':
                    return KeyboardKey::Z;
                case VK_OEM_4:
                    return KeyboardKey::LeftBracket;
                case VK_OEM_5:
                    return KeyboardKey::Backslash;
                case VK_OEM_6:
                    return KeyboardKey::RightBracket;
                case VK_OEM_3:
                    return KeyboardKey::GraveAccent;
                case VK_ESCAPE:
                    return KeyboardKey::Escape;
                case VK_RETURN:
                    return KeyboardKey::Enter;
                case VK_TAB:
                    return KeyboardKey::Tab;
                case VK_BACK:
                    return KeyboardKey::Backspace;
                case VK_INSERT:
                    return KeyboardKey::Insert;
                case VK_DELETE:
                    return KeyboardKey::Delete;
                case VK_RIGHT:
                    return KeyboardKey::Right;
                case VK_LEFT:
                    return KeyboardKey::Left;
                case VK_DOWN:
                    return KeyboardKey::Down;
                case VK_UP:
                    return KeyboardKey::Up;
                case VK_PRIOR:
                    return KeyboardKey::PageUp;
                case VK_NEXT:
                    return KeyboardKey::PageDown;
                case VK_HOME:
                    return KeyboardKey::Home;
                case VK_END:
                    return KeyboardKey::End;
                case VK_CAPITAL:
                    return KeyboardKey::CapsLock;
                case VK_SCROLL:
                    return KeyboardKey::ScrollLock;
                case VK_NUMLOCK:
                    return KeyboardKey::NumLock;
                case VK_SNAPSHOT:
                    return KeyboardKey::PrintScreen;
                case VK_PAUSE:
                    return KeyboardKey::Pause;
                case VK_F1:
                    return KeyboardKey::F1;
                case VK_F2:
                    return KeyboardKey::F2;
                case VK_F3:
                    return KeyboardKey::F3;
                case VK_F4:
                    return KeyboardKey::F4;
                case VK_F5:
                    return KeyboardKey::F5;
                case VK_F6:
                    return KeyboardKey::F6;
                case VK_F7:
                    return KeyboardKey::F7;
                case VK_F8:
                    return KeyboardKey::F8;
                case VK_F9:
                    return KeyboardKey::F9;
                case VK_F10:
                    return KeyboardKey::F10;
                case VK_F11:
                    return KeyboardKey::F11;
                case VK_F12:
                    return KeyboardKey::F12;
                case VK_NUMPAD0:
                    return KeyboardKey::Kp0;
                case VK_NUMPAD1:
                    return KeyboardKey::Kp1;
                case VK_NUMPAD2:
                    return KeyboardKey::Kp2;
                case VK_NUMPAD3:
                    return KeyboardKey::Kp3;
                case VK_NUMPAD4:
                    return KeyboardKey::Kp4;
                case VK_NUMPAD5:
                    return KeyboardKey::Kp5;
                case VK_NUMPAD6:
                    return KeyboardKey::Kp6;
                case VK_NUMPAD7:
                    return KeyboardKey::Kp7;
                case VK_NUMPAD8:
                    return KeyboardKey::Kp8;
                case VK_NUMPAD9:
                    return KeyboardKey::Kp9;
                case VK_DECIMAL:
                    return KeyboardKey::KpDecimal;
                case VK_DIVIDE:
                    return KeyboardKey::KpDivide;
                case VK_MULTIPLY:
                    return KeyboardKey::KpMultiply;
                case VK_SUBTRACT:
                    return KeyboardKey::KpSubtract;
                case VK_ADD:
                    return KeyboardKey::KpAdd;
                case VK_LSHIFT:
                    return KeyboardKey::LeftShift;
                case VK_LCONTROL:
                    return KeyboardKey::LeftControl;
                case VK_LMENU:
                    return KeyboardKey::LeftAlt;
                case VK_LWIN:
                    return KeyboardKey::LeftSuper;
                case VK_RSHIFT:
                    return KeyboardKey::RightShift;
                case VK_RCONTROL:
                    return KeyboardKey::RightControl;
                case VK_RMENU:
                    return KeyboardKey::RightAlt;
                case VK_RWIN:
                    return KeyboardKey::RightSuper;
                case VK_APPS:
                    return KeyboardKey::Menu;
                default:
                    return KeyboardKey::Unknown;
            }
        }

        static MouseButton wparam_to_mouse_button(WPARAM wparam) {
            switch (GET_XBUTTON_WPARAM(wparam)) {
                case XBUTTON1:
                    return MouseButton::Button4;
                case XBUTTON2:
                    return MouseButton::Button5;
                default:
                    return MouseButton::Unknown;
            }
        }

        static KeyModifiers get_key_modifiers() {
            KeyModifiers mods = KeyModifiers::None;
            if (GetKeyState(VK_SHIFT) & 0x8000)
                mods = mods | KeyModifiers::Shift;
            if (GetKeyState(VK_CONTROL) & 0x8000)
                mods = mods | KeyModifiers::Control;
            if (GetKeyState(VK_MENU) & 0x8000)
                mods = mods | KeyModifiers::Alt;
            if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
                mods = mods | KeyModifiers::Super;
            if (GetKeyState(VK_CAPITAL) & 1)
                mods = mods | KeyModifiers::CapsLock;
            if (GetKeyState(VK_NUMLOCK) & 1)
                mods = mods | KeyModifiers::NumLock;
            return mods;
        }

    } // namespace

    Win32WindowBackend::Win32WindowBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus)
        : WindowBackend(std::move(windowEventBus)),
          m_windowClassName("RibbleWin32Window") {}

    Win32WindowBackend::~Win32WindowBackend() { Win32WindowBackend::shutdown(); }

    void Win32WindowBackend::register_window_class() {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = wnd_proc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = m_windowClassName.c_str();

        if (!RegisterClassExA(&wc)) {
            RIBBLE_LOG_ERROR("RegisterClassExA failed: {}", GetLastError());
        }
    }

    void Win32WindowBackend::unregister_window_class() {
        UnregisterClassA(m_windowClassName.c_str(), GetModuleHandle(nullptr));
    }

    LRESULT CALLBACK Win32WindowBackend::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        auto *self = reinterpret_cast<Win32WindowBackend *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        if (!self) {
            if (msg == WM_CREATE) {
                auto *cs = reinterpret_cast<CREATESTRUCTA *>(lparam);
                self = reinterpret_cast<Win32WindowBackend *>(cs->lpCreateParams);
                SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            }
            return DefWindowProcA(hwnd, msg, wparam, lparam);
        }

        switch (msg) {
            case WM_CLOSE:
                self->m_shouldClose = true;
                self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE: {
                int w = LOWORD(lparam);
                int h = HIWORD(lparam);
                if (w > 0 && h > 0) {
                    self->m_width = w;
                    self->m_height = h;
                    self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowResizeEvent>(w, h));
                }
                break;
            }

            case WM_MOVE: {
                int x = LOWORD(lparam);
                int y = HIWORD(lparam);
                self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowMoveEvent>(x, y));
                break;
            }

            case WM_SETFOCUS:
                self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(true));
                break;

            case WM_KILLFOCUS:
                self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(false));
                break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                const bool repeat = (lparam & (1 << 30)) != 0;
                const int scancode = (lparam >> 16) & 0x1FF;
                const KeyboardKey key = vk_to_ribble(static_cast<int>(wparam));
                const KeyModifiers mods = get_key_modifiers();
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<KeyDownEvent>(key, mods, scancode, repeat));
                break;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP: {
                const int scancode = (lparam >> 16) & 0x1FF;
                const KeyboardKey key = vk_to_ribble(static_cast<int>(wparam));
                const KeyModifiers mods = get_key_modifiers();
                self->m_windowEventBus->dispatch_immediate(std::make_shared<KeyUpEvent>(key, mods, scancode));
                break;
            }

            case WM_CHAR: {
                uint32_t codepoint = static_cast<uint32_t>(wparam);
                if (codepoint >= 0x20 && codepoint != 0x7F) { // Avoid control chars
                    self->m_windowEventBus->dispatch_immediate(std::make_shared<TextInputEvent>(codepoint));
                }
                break;
            }

            case WM_MOUSEMOVE: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                double dx = 0, dy = 0;
                if (self->m_firstMouseMove) {
                    self->m_firstMouseMove = false;
                } else {
                    dx = x - self->m_lastMouseX;
                    dy = y - self->m_lastMouseY;
                }
                self->m_lastMouseX = x;
                self->m_lastMouseY = y;
                self->m_windowEventBus->dispatch_immediate(std::make_shared<MouseMoveEvent>(x, y, dx, dy));
                break;
            }

            case WM_LBUTTONDOWN: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Left, ButtonAction::Press,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_LBUTTONUP: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Left, ButtonAction::Release,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_RBUTTONDOWN: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Right, ButtonAction::Press,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_RBUTTONUP: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Right, ButtonAction::Release,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_MBUTTONDOWN: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Middle, ButtonAction::Press,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_MBUTTONUP: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                self->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(MouseButton::Middle, ButtonAction::Release,
                                                       get_key_modifiers(), x, y));
                break;
            }
            case WM_XBUTTONDOWN: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                MouseButton btn = wparam_to_mouse_button(wparam);
                if (btn != MouseButton::Unknown)
                    self->m_windowEventBus->dispatch_immediate(
                        std::make_shared<MouseButtonEvent>(btn, ButtonAction::Press,
                                                           get_key_modifiers(), x, y));
                break;
            }
            case WM_XBUTTONUP: {
                const double x = static_cast<double>(GET_X_LPARAM(lparam));
                const double y = static_cast<double>(GET_Y_LPARAM(lparam));
                MouseButton btn = wparam_to_mouse_button(wparam);
                if (btn != MouseButton::Unknown)
                    self->m_windowEventBus->dispatch_immediate(
                        std::make_shared<MouseButtonEvent>(btn, ButtonAction::Release,
                                                           get_key_modifiers(), x, y));
                break;
            }

            case WM_MOUSEWHEEL: {
                const double delta = static_cast<double>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<double>(WHEEL_DELTA);
                // Standard: positive = scroll up, negative = scroll down
                self->m_windowEventBus->dispatch_immediate(std::make_shared<MouseScrollEvent>(0, delta));
                break;
            }

            case WM_MOUSEHWHEEL: {
                const double delta = static_cast<double>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<double>(WHEEL_DELTA);
                self->m_windowEventBus->dispatch_immediate(std::make_shared<MouseScrollEvent>(delta, 0));
                break;
            }

            default:
                return DefWindowProcA(hwnd, msg, wparam, lparam);
        }
        return 0;
    }

    Result<void, WindowBackend::Failure> Win32WindowBackend::initialize(int width, int height, const char *title) {
        WindowBackend::initialize(width, height, title);
        m_width = width;
        m_height = height;

        register_window_class();

        RECT rect = {0, 0, width, height};
        if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE)) {
            unregister_window_class();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "AdjustWindowRect failed"));
        }

        m_hwnd = CreateWindowExA(
            0, m_windowClassName.c_str(), title ? title : "Ribble",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, GetModuleHandle(nullptr), this);

        if (!m_hwnd) {
            unregister_window_class();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure,
                                    "CreateWindowExA failed: {}", GetLastError()));
        }

        m_hdc = GetDC(m_hwnd);
        if (!m_hdc) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
            unregister_window_class();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "GetDC failed"));
        }

        ShowWindow(m_hwnd, SW_SHOW);

        // Dispatch initial size so WindowContext has correct framebuffer dimensions
        m_windowEventBus->dispatch_immediate(std::make_shared<WindowResizeEvent>(width, height));

        RIBBLE_LOG_INFO("Win32 window created: {}x{}", width, height);
        return Ok();
    }

    Result<void, WindowBackend::Failure> Win32WindowBackend::poll_events() {
        MSG msg;
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_shouldClose = true;
                m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return Ok();
    }

    Result<void, WindowBackend::Failure> Win32WindowBackend::shutdown() {
        if (m_hdc && m_hwnd) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        unregister_window_class();
        RIBBLE_LOG_INFO("Win32 window closed.");
        return Ok();
    }

} // namespace backend
