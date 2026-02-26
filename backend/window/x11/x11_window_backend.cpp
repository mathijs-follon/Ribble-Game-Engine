// Include our event types before X11 so X11 macros don't replace our enum identifiers
#include "../../common/window_events.h"

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <ribble/core/logger.h>
#include "x11_window_backend.h"

// Undef X11 macros that conflict with our enums (KeyModifiers, MouseButton)
#undef None
#undef Success
#undef Failure
#undef Button1
#undef Button2
#undef Button3
#undef Button4
#undef Button5
#undef Shift
#undef Control
#undef Alt
#undef Super
#undef CapsLock
#undef NumLock

using namespace ribble::core;

namespace backend {

    // Convert X11 keysym to KeyboardKey
    static KeyboardKey x11_keysym_to_ribble(KeySym keysym) {
        switch (keysym) {
            case XK_space:
                return KeyboardKey::Space;
            case XK_apostrophe:
                return KeyboardKey::Apostrophe;
            case XK_comma:
                return KeyboardKey::Comma;
            case XK_minus:
                return KeyboardKey::Minus;
            case XK_period:
                return KeyboardKey::Period;
            case XK_slash:
                return KeyboardKey::Slash;
            case XK_0:
                return KeyboardKey::Num0;
            case XK_1:
                return KeyboardKey::Num1;
            case XK_2:
                return KeyboardKey::Num2;
            case XK_3:
                return KeyboardKey::Num3;
            case XK_4:
                return KeyboardKey::Num4;
            case XK_5:
                return KeyboardKey::Num5;
            case XK_6:
                return KeyboardKey::Num6;
            case XK_7:
                return KeyboardKey::Num7;
            case XK_8:
                return KeyboardKey::Num8;
            case XK_9:
                return KeyboardKey::Num9;
            case XK_semicolon:
                return KeyboardKey::Semicolon;
            case XK_equal:
                return KeyboardKey::Equal;
            case XK_a:
            case XK_A:
                return KeyboardKey::A;
            case XK_b:
            case XK_B:
                return KeyboardKey::B;
            case XK_c:
            case XK_C:
                return KeyboardKey::C;
            case XK_d:
            case XK_D:
                return KeyboardKey::D;
            case XK_e:
            case XK_E:
                return KeyboardKey::E;
            case XK_f:
            case XK_F:
                return KeyboardKey::F;
            case XK_g:
            case XK_G:
                return KeyboardKey::G;
            case XK_h:
            case XK_H:
                return KeyboardKey::H;
            case XK_i:
            case XK_I:
                return KeyboardKey::I;
            case XK_j:
            case XK_J:
                return KeyboardKey::J;
            case XK_k:
            case XK_K:
                return KeyboardKey::K;
            case XK_l:
            case XK_L:
                return KeyboardKey::L;
            case XK_m:
            case XK_M:
                return KeyboardKey::M;
            case XK_n:
            case XK_N:
                return KeyboardKey::N;
            case XK_o:
            case XK_O:
                return KeyboardKey::O;
            case XK_p:
            case XK_P:
                return KeyboardKey::P;
            case XK_q:
            case XK_Q:
                return KeyboardKey::Q;
            case XK_r:
            case XK_R:
                return KeyboardKey::R;
            case XK_s:
            case XK_S:
                return KeyboardKey::S;
            case XK_t:
            case XK_T:
                return KeyboardKey::T;
            case XK_u:
            case XK_U:
                return KeyboardKey::U;
            case XK_v:
            case XK_V:
                return KeyboardKey::V;
            case XK_w:
            case XK_W:
                return KeyboardKey::W;
            case XK_x:
            case XK_X:
                return KeyboardKey::X;
            case XK_y:
            case XK_Y:
                return KeyboardKey::Y;
            case XK_z:
            case XK_Z:
                return KeyboardKey::Z;
            case XK_bracketleft:
                return KeyboardKey::LeftBracket;
            case XK_backslash:
                return KeyboardKey::Backslash;
            case XK_bracketright:
                return KeyboardKey::RightBracket;
            case XK_grave:
                return KeyboardKey::GraveAccent;
            case XK_F1:
                return KeyboardKey::F1;
            case XK_F2:
                return KeyboardKey::F2;
            case XK_F3:
                return KeyboardKey::F3;
            case XK_F4:
                return KeyboardKey::F4;
            case XK_F5:
                return KeyboardKey::F5;
            case XK_F6:
                return KeyboardKey::F6;
            case XK_F7:
                return KeyboardKey::F7;
            case XK_F8:
                return KeyboardKey::F8;
            case XK_F9:
                return KeyboardKey::F9;
            case XK_F10:
                return KeyboardKey::F10;
            case XK_F11:
                return KeyboardKey::F11;
            case XK_F12:
                return KeyboardKey::F12;
            case XK_Escape:
                return KeyboardKey::Escape;
            case XK_Return:
            case XK_KP_Enter:
                return KeyboardKey::Enter;
            case XK_Tab:
                return KeyboardKey::Tab;
            case XK_BackSpace:
                return KeyboardKey::Backspace;
            case XK_Insert:
                return KeyboardKey::Insert;
            case XK_Delete:
                return KeyboardKey::Delete;
            case XK_Right:
                return KeyboardKey::Right;
            case XK_Left:
                return KeyboardKey::Left;
            case XK_Down:
                return KeyboardKey::Down;
            case XK_Up:
                return KeyboardKey::Up;
            case XK_Page_Up:
                return KeyboardKey::PageUp;
            case XK_Page_Down:
                return KeyboardKey::PageDown;
            case XK_Home:
                return KeyboardKey::Home;
            case XK_End:
                return KeyboardKey::End;
            case XK_Caps_Lock:
                return KeyboardKey::CapsLock;
            case XK_Scroll_Lock:
                return KeyboardKey::ScrollLock;
            case XK_Num_Lock:
                return KeyboardKey::NumLock;
            case XK_Print:
                return KeyboardKey::PrintScreen;
            case XK_Pause:
                return KeyboardKey::Pause;
            case XK_KP_0:
                return KeyboardKey::Kp0;
            case XK_KP_1:
                return KeyboardKey::Kp1;
            case XK_KP_2:
                return KeyboardKey::Kp2;
            case XK_KP_3:
                return KeyboardKey::Kp3;
            case XK_KP_4:
                return KeyboardKey::Kp4;
            case XK_KP_5:
                return KeyboardKey::Kp5;
            case XK_KP_6:
                return KeyboardKey::Kp6;
            case XK_KP_7:
                return KeyboardKey::Kp7;
            case XK_KP_8:
                return KeyboardKey::Kp8;
            case XK_KP_9:
                return KeyboardKey::Kp9;
            case XK_KP_Decimal:
                return KeyboardKey::KpDecimal;
            case XK_KP_Divide:
                return KeyboardKey::KpDivide;
            case XK_KP_Multiply:
                return KeyboardKey::KpMultiply;
            case XK_KP_Subtract:
                return KeyboardKey::KpSubtract;
            case XK_KP_Add:
                return KeyboardKey::KpAdd;
            case XK_KP_Equal:
                return KeyboardKey::KpEqual;
            case XK_Shift_L:
                return KeyboardKey::LeftShift;
            case XK_Control_L:
                return KeyboardKey::LeftControl;
            case XK_Alt_L:
                return KeyboardKey::LeftAlt;
            case XK_Super_L:
                return KeyboardKey::LeftSuper;
            case XK_Shift_R:
                return KeyboardKey::RightShift;
            case XK_Control_R:
                return KeyboardKey::RightControl;
            case XK_Alt_R:
                return KeyboardKey::RightAlt;
            case XK_Super_R:
                return KeyboardKey::RightSuper;
            case XK_Menu:
                return KeyboardKey::Menu;
            default:
                return KeyboardKey::Unknown;
        }
    }

    static MouseButton x11_button_to_ribble(unsigned int button) {
        switch (button) {
            case 1:
                return MouseButton::Left;
            case 2:
                return MouseButton::Middle;
            case 3:
                return MouseButton::Right;
            case 4:
                return MouseButton::Button4;
            case 5:
                return MouseButton::Button5;
            default:
                return MouseButton::Unknown;
        }
    }

    static KeyModifiers x11_state_to_ribble(unsigned int state) {
        KeyModifiers out = KeyModifiers::None;
        if (state & ShiftMask)
            out = out | KeyModifiers::Shift;
        if (state & ControlMask)
            out = out | KeyModifiers::Control;
        if (state & Mod1Mask)
            out = out | KeyModifiers::Alt; // Mod1 is typically Alt
        if (state & Mod4Mask)
            out = out | KeyModifiers::Super; // Mod4 is typically Super/Windows
        if (state & LockMask)
            out = out | KeyModifiers::CapsLock;
        if (state & Mod2Mask)
            out = out | KeyModifiers::NumLock; // Mod2 is typically NumLock
        return out;
    }

    ribble::core::Result<void, WindowBackend::Failure> X11Window::initialize(int width, int height, const char *title) {
        WindowBackend::initialize(width, height, title);

        // Open connection to X server
        m_display = XOpenDisplay(nullptr);
        if (!m_display) {
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "XOpenDisplay failed"));
        }
        m_x11Initialized = true;
        RIBBLE_LOG_INFO("X11 display opened.");

        m_screen = DefaultScreen(m_display);
        Window root = RootWindow(m_display, m_screen);

        // Create window
        m_window = XCreateSimpleWindow(m_display, root, 0, 0, // x, y
                                       static_cast<unsigned int>(width), static_cast<unsigned int>(height),
                                       0, // border width
                                       BlackPixel(m_display, m_screen), // border
                                       WhitePixel(m_display, m_screen) // background
        );

        if (!m_window) {
            XCloseDisplay(m_display);
            m_display = nullptr;
            m_x11Initialized = false;
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "XCreateSimpleWindow failed"));
        }

        // Set window title
        XStoreName(m_display, m_window, title);

        // Set window protocols (for close button)
        Atom wmDelete = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_display, m_window, &wmDelete, 1);

        // Select events we want to receive
        XSelectInput(m_display, m_window,
                     ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
                             PointerMotionMask | StructureNotifyMask | FocusChangeMask);

        // Map the window (make it visible)
        XMapWindow(m_display, m_window);
        XFlush(m_display);

        RIBBLE_LOG_INFO("X11 window created: {}x{}", width, height);

        return Ok();
    }

    ribble::core::Result<void, WindowBackend::Failure> X11Window::poll_events() {
        if (!m_display) {
            return Ok();
        }

        // Check for pending events
        while (XPending(m_display) > 0) {
            XEvent event;
            XNextEvent(m_display, &event);

            switch (event.type) {
                case ClientMessage: {
                    // Check for window close
                    Atom wmDelete = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
                    if (event.xclient.message_type == XInternAtom(m_display, "WM_PROTOCOLS", False) &&
                        static_cast<Atom>(event.xclient.data.l[0]) == wmDelete) {
                        m_shouldClose = true;
                        m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
                    }
                    break;
                }

                case ConfigureNotify: {
                    XConfigureEvent &ce = event.xconfigure;
                    if (ce.width > 0 && ce.height > 0) {
                        m_windowEventBus->dispatch_immediate(std::make_shared<WindowResizeEvent>(ce.width, ce.height));
                    }
                    break;
                }

                case KeyPress: {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    KeyboardKey key = x11_keysym_to_ribble(keysym);
                    KeyModifiers mods = x11_state_to_ribble(event.xkey.state);
                    int scancode = event.xkey.keycode;

                    // Check for key repeat (X11 doesn't have a built-in repeat flag, so we track it)
                    // For simplicity, we'll set repeat to false here
                    // A more sophisticated implementation would track key states
                    m_windowEventBus->dispatch_immediate(std::make_shared<KeyDownEvent>(key, mods, scancode, false));
                    break;
                }

                case KeyRelease: {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    KeyboardKey key = x11_keysym_to_ribble(keysym);
                    KeyModifiers mods = x11_state_to_ribble(event.xkey.state);
                    int scancode = event.xkey.keycode;

                    m_windowEventBus->dispatch_immediate(std::make_shared<KeyUpEvent>(key, mods, scancode));
                    break;
                }

                case ButtonPress:
                case ButtonRelease: {
                    MouseButton button = x11_button_to_ribble(event.xbutton.button);
                    KeyModifiers mods = x11_state_to_ribble(event.xbutton.state);
                    double x = static_cast<double>(event.xbutton.x);
                    double y = static_cast<double>(event.xbutton.y);
                    ButtonAction action = (event.type == ButtonPress) ? ButtonAction::Press : ButtonAction::Release;

                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<MouseButtonEvent>(button, action, mods, x, y));
                    break;
                }

                case MotionNotify: {
                    XMotionEvent &me = event.xmotion;
                    double x = static_cast<double>(me.x);
                    double y = static_cast<double>(me.y);
                    double dx = 0.0, dy = 0.0;

                    if (m_firstMouseMove) {
                        m_firstMouseMove = false;
                    } else {
                        dx = x - m_lastMouseX;
                        dy = y - m_lastMouseY;
                    }
                    m_lastMouseX = x;
                    m_lastMouseY = y;

                    m_windowEventBus->dispatch_immediate(std::make_shared<MouseMoveEvent>(x, y, dx, dy));
                    break;
                }

                case FocusIn:
                case FocusOut: {
                    bool focused = (event.type == FocusIn);
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(focused));
                    break;
                }

                default:
                    break;
            }
        }

        return Ok();
    }

    ribble::core::Result<void, WindowBackend::Failure> X11Window::shutdown() {
        if (m_window) {
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }

        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
            m_x11Initialized = false;
        }

        return Ok();
    }

    void *X11Window::native_handle() const { return reinterpret_cast<void *>(m_window); }

} // namespace backend
