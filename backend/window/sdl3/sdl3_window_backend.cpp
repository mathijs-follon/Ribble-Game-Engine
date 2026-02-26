#include "sdl3_window_backend.h"

#include "../../common/window_events.h"

using namespace ribble::core;

namespace backend {

    static KeyModifiers sdl_mod_to_ribble(SDL_Keymod mod) {
        KeyModifiers out = KeyModifiers::None;
        if (mod & SDL_KMOD_SHIFT)
            out = out | KeyModifiers::Shift;
        if (mod & SDL_KMOD_CTRL)
            out = out | KeyModifiers::Control;
        if (mod & SDL_KMOD_ALT)
            out = out | KeyModifiers::Alt;
        if (mod & SDL_KMOD_GUI)
            out = out | KeyModifiers::Super;
        if (mod & SDL_KMOD_CAPS)
            out = out | KeyModifiers::CapsLock;
        if (mod & SDL_KMOD_NUM)
            out = out | KeyModifiers::NumLock;
        return out;
    }

    static MouseButton sdl_mouse_button_to_ribble(uint8_t btn) {
        switch (btn) {
            case SDL_BUTTON_LEFT:
                return MouseButton::Left;
            case SDL_BUTTON_RIGHT:
                return MouseButton::Right;
            case SDL_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case SDL_BUTTON_X1:
                return MouseButton::Button4;
            case SDL_BUTTON_X2:
                return MouseButton::Button5;
            default:
                return MouseButton::Unknown;
        }
    }

    static GamepadButton sdl_gamepad_button_to_ribble(SDL_GamepadButton btn) {
        switch (btn) {
            case SDL_GAMEPAD_BUTTON_SOUTH:
                return GamepadButton::South;
            case SDL_GAMEPAD_BUTTON_EAST:
                return GamepadButton::East;
            case SDL_GAMEPAD_BUTTON_WEST:
                return GamepadButton::West;
            case SDL_GAMEPAD_BUTTON_NORTH:
                return GamepadButton::North;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
                return GamepadButton::LeftBumper;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
                return GamepadButton::RightBumper;
            case SDL_GAMEPAD_BUTTON_BACK:
                return GamepadButton::Select;
            case SDL_GAMEPAD_BUTTON_START:
                return GamepadButton::Start;
            case SDL_GAMEPAD_BUTTON_GUIDE:
                return GamepadButton::Guide;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK:
                return GamepadButton::LeftThumb;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
                return GamepadButton::RightThumb;
            case SDL_GAMEPAD_BUTTON_DPAD_UP:
                return GamepadButton::DpadUp;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
                return GamepadButton::DpadDown;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
                return GamepadButton::DpadLeft;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
                return GamepadButton::DpadRight;
            default:
                return GamepadButton::Unknown;
        }
    }

    static GamepadAxis sdl_gamepad_axis_to_ribble(SDL_GamepadAxis axis) {
        switch (axis) {
            case SDL_GAMEPAD_AXIS_LEFTX:
                return GamepadAxis::LeftX;
            case SDL_GAMEPAD_AXIS_LEFTY:
                return GamepadAxis::LeftY;
            case SDL_GAMEPAD_AXIS_RIGHTX:
                return GamepadAxis::RightX;
            case SDL_GAMEPAD_AXIS_RIGHTY:
                return GamepadAxis::RightY;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
                return GamepadAxis::LeftTrigger;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
                return GamepadAxis::RightTrigger;
            default:
                return GamepadAxis::Unknown;
        }
    }

    static KeyboardKey sdl_keycode_to_ribble(SDL_Keycode key) {
        switch (key) {
            case SDLK_SPACE:
                return KeyboardKey::Space;
            case SDLK_APOSTROPHE:
                return KeyboardKey::Apostrophe;
            case SDLK_COMMA:
                return KeyboardKey::Comma;
            case SDLK_MINUS:
                return KeyboardKey::Minus;
            case SDLK_PERIOD:
                return KeyboardKey::Period;
            case SDLK_SLASH:
                return KeyboardKey::Slash;
            case SDLK_0:
                return KeyboardKey::Num0;
            case SDLK_1:
                return KeyboardKey::Num1;
            case SDLK_2:
                return KeyboardKey::Num2;
            case SDLK_3:
                return KeyboardKey::Num3;
            case SDLK_4:
                return KeyboardKey::Num4;
            case SDLK_5:
                return KeyboardKey::Num5;
            case SDLK_6:
                return KeyboardKey::Num6;
            case SDLK_7:
                return KeyboardKey::Num7;
            case SDLK_8:
                return KeyboardKey::Num8;
            case SDLK_9:
                return KeyboardKey::Num9;
            case SDLK_SEMICOLON:
                return KeyboardKey::Semicolon;
            case SDLK_EQUALS:
                return KeyboardKey::Equal;
            case SDLK_A:
                return KeyboardKey::A;
            case SDLK_B:
                return KeyboardKey::B;
            case SDLK_C:
                return KeyboardKey::C;
            case SDLK_D:
                return KeyboardKey::D;
            case SDLK_E:
                return KeyboardKey::E;
            case SDLK_F:
                return KeyboardKey::F;
            case SDLK_G:
                return KeyboardKey::G;
            case SDLK_H:
                return KeyboardKey::H;
            case SDLK_I:
                return KeyboardKey::I;
            case SDLK_J:
                return KeyboardKey::J;
            case SDLK_K:
                return KeyboardKey::K;
            case SDLK_L:
                return KeyboardKey::L;
            case SDLK_M:
                return KeyboardKey::M;
            case SDLK_N:
                return KeyboardKey::N;
            case SDLK_O:
                return KeyboardKey::O;
            case SDLK_P:
                return KeyboardKey::P;
            case SDLK_Q:
                return KeyboardKey::Q;
            case SDLK_R:
                return KeyboardKey::R;
            case SDLK_S:
                return KeyboardKey::S;
            case SDLK_T:
                return KeyboardKey::T;
            case SDLK_U:
                return KeyboardKey::U;
            case SDLK_V:
                return KeyboardKey::V;
            case SDLK_W:
                return KeyboardKey::W;
            case SDLK_X:
                return KeyboardKey::X;
            case SDLK_Y:
                return KeyboardKey::Y;
            case SDLK_Z:
                return KeyboardKey::Z;
            case SDLK_LEFTBRACKET:
                return KeyboardKey::LeftBracket;
            case SDLK_BACKSLASH:
                return KeyboardKey::Backslash;
            case SDLK_RIGHTBRACKET:
                return KeyboardKey::RightBracket;
            case SDLK_GRAVE:
                return KeyboardKey::GraveAccent;
            case SDLK_ESCAPE:
                return KeyboardKey::Escape;
            case SDLK_RETURN:
                return KeyboardKey::Enter;
            case SDLK_TAB:
                return KeyboardKey::Tab;
            case SDLK_BACKSPACE:
                return KeyboardKey::Backspace;
            case SDLK_INSERT:
                return KeyboardKey::Insert;
            case SDLK_DELETE:
                return KeyboardKey::Delete;
            case SDLK_RIGHT:
                return KeyboardKey::Right;
            case SDLK_LEFT:
                return KeyboardKey::Left;
            case SDLK_DOWN:
                return KeyboardKey::Down;
            case SDLK_UP:
                return KeyboardKey::Up;
            case SDLK_PAGEUP:
                return KeyboardKey::PageUp;
            case SDLK_PAGEDOWN:
                return KeyboardKey::PageDown;
            case SDLK_HOME:
                return KeyboardKey::Home;
            case SDLK_END:
                return KeyboardKey::End;
            case SDLK_CAPSLOCK:
                return KeyboardKey::CapsLock;
            case SDLK_SCROLLLOCK:
                return KeyboardKey::ScrollLock;
            case SDLK_NUMLOCKCLEAR:
                return KeyboardKey::NumLock;
            case SDLK_PRINTSCREEN:
                return KeyboardKey::PrintScreen;
            case SDLK_PAUSE:
                return KeyboardKey::Pause;
            case SDLK_F1:
                return KeyboardKey::F1;
            case SDLK_F2:
                return KeyboardKey::F2;
            case SDLK_F3:
                return KeyboardKey::F3;
            case SDLK_F4:
                return KeyboardKey::F4;
            case SDLK_F5:
                return KeyboardKey::F5;
            case SDLK_F6:
                return KeyboardKey::F6;
            case SDLK_F7:
                return KeyboardKey::F7;
            case SDLK_F8:
                return KeyboardKey::F8;
            case SDLK_F9:
                return KeyboardKey::F9;
            case SDLK_F10:
                return KeyboardKey::F10;
            case SDLK_F11:
                return KeyboardKey::F11;
            case SDLK_F12:
                return KeyboardKey::F12;
            case SDLK_KP_0:
                return KeyboardKey::Kp0;
            case SDLK_KP_1:
                return KeyboardKey::Kp1;
            case SDLK_KP_2:
                return KeyboardKey::Kp2;
            case SDLK_KP_3:
                return KeyboardKey::Kp3;
            case SDLK_KP_4:
                return KeyboardKey::Kp4;
            case SDLK_KP_5:
                return KeyboardKey::Kp5;
            case SDLK_KP_6:
                return KeyboardKey::Kp6;
            case SDLK_KP_7:
                return KeyboardKey::Kp7;
            case SDLK_KP_8:
                return KeyboardKey::Kp8;
            case SDLK_KP_9:
                return KeyboardKey::Kp9;
            case SDLK_KP_PERIOD:
                return KeyboardKey::KpDecimal;
            case SDLK_KP_DIVIDE:
                return KeyboardKey::KpDivide;
            case SDLK_KP_MULTIPLY:
                return KeyboardKey::KpMultiply;
            case SDLK_KP_MINUS:
                return KeyboardKey::KpSubtract;
            case SDLK_KP_PLUS:
                return KeyboardKey::KpAdd;
            case SDLK_KP_ENTER:
                return KeyboardKey::KpEnter;
            case SDLK_KP_EQUALS:
                return KeyboardKey::KpEqual;
            case SDLK_LSHIFT:
                return KeyboardKey::LeftShift;
            case SDLK_LCTRL:
                return KeyboardKey::LeftControl;
            case SDLK_LALT:
                return KeyboardKey::LeftAlt;
            case SDLK_LGUI:
                return KeyboardKey::LeftSuper;
            case SDLK_RSHIFT:
                return KeyboardKey::RightShift;
            case SDLK_RCTRL:
                return KeyboardKey::RightControl;
            case SDLK_RALT:
                return KeyboardKey::RightAlt;
            case SDLK_RGUI:
                return KeyboardKey::RightSuper;
            case SDLK_MENU:
                return KeyboardKey::Menu;
            default:
                return KeyboardKey::Unknown;
        }
    }

    Result<void, WindowBackend::Failure> SDLWindow::initialize(int width, int height, const char *title) {
        WindowBackend::initialize(width, height, title);

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            return Fail(
                    RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "SDL_Init failed: {}", SDL_GetError()));
        }
        RIBBLE_LOG_INFO("SDL_Init success. Video driver: {}",
                        SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "null");

        uint32_t windowFlags = SDL_WINDOW_RESIZABLE;
        if (m_graphicsAPI == GraphicsAPI::Vulkan) {
            windowFlags |= SDL_WINDOW_VULKAN;
        } else {
            // Set OpenGL attributes before creating the window
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#if defined(RIBBLE_DEBUG)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
            windowFlags |= SDL_WINDOW_OPENGL;
        }

        m_window = SDL_CreateWindow(title, width, height, windowFlags);
        if (!m_window) {
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "SDL_CreateWindow failed: {}",
                                     SDL_GetError()));
        }
        RIBBLE_LOG_INFO("SDL_CreateWindow success.");

        if (!SDL_ShowWindow(m_window)) {
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "SDL_ShowWindow failed: {}",
                                     SDL_GetError()));
        }
        RIBBLE_LOG_INFO("SDL_ShowWindow success.");

        SDL_RaiseWindow(m_window);

        SDL_PumpEvents();

        return Ok();
    }

    Result<void, WindowBackend::Failure> SDLWindow::poll_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

                case SDL_EVENT_QUIT:
                    m_shouldClose = true;
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<WindowResizeEvent>(event.window.data1, event.window.data2));
                    break;

                case SDL_EVENT_WINDOW_MOVED:
                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<WindowMoveEvent>(event.window.data1, event.window.data2));
                    break;

                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(true));
                    break;

                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(false));
                    break;

                case SDL_EVENT_WINDOW_MINIMIZED:
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowMinimizeEvent>(true));
                    break;

                case SDL_EVENT_WINDOW_RESTORED:
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowMinimizeEvent>(false));
                    break;

                case SDL_EVENT_WINDOW_MAXIMIZED:
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowMaximizeEvent>(true));
                    break;

                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                    // Re-query scale in case DPI changed
                    float scale = SDL_GetWindowDisplayScale(m_window);
                    m_windowEventBus->dispatch_immediate(std::make_shared<WindowContentScaleEvent>(scale, scale));
                    break;
                }

                // ── Keyboard ──────────────────────────────────────────────────────────
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP: {
                    const bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
                    const bool repeated = pressed && (event.key.repeat);
                    const KeyboardKey key = sdl_keycode_to_ribble(event.key.key);
                    const KeyModifiers mods = sdl_mod_to_ribble(event.key.mod);

                    if (pressed) {
                        m_windowEventBus->dispatch_immediate(
                                std::make_shared<KeyDownEvent>(key, mods, event.key.scancode, repeated));
                    } else {
                        m_windowEventBus->dispatch_immediate(
                                std::make_shared<KeyUpEvent>(key, mods, event.key.scancode));
                    }
                    break;
                }

                case SDL_EVENT_TEXT_INPUT: {
                    // SDL gives us UTF-8; decode first code point
                    const auto *utf8 = reinterpret_cast<const uint8_t *>(event.text.text);
                    uint32_t codepoint = 0;
                    if (utf8[0] < 0x80)
                        codepoint = utf8[0];
                    else if (utf8[0] < 0xE0)
                        codepoint = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
                    else if (utf8[0] < 0xF0)
                        codepoint = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
                    else
                        codepoint = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) |
                                    (utf8[3] & 0x3F);
                    m_windowEventBus->dispatch_immediate(std::make_shared<TextInputEvent>(codepoint));
                    break;
                }

                case SDL_EVENT_MOUSE_MOTION:
                    m_windowEventBus->dispatch_immediate(std::make_shared<MouseMoveEvent>(
                            event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel));
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    const ButtonAction action =
                            (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? ButtonAction::Press : ButtonAction::Release;
                    m_windowEventBus->dispatch_immediate(std::make_shared<MouseButtonEvent>(
                            sdl_mouse_button_to_ribble(event.button.button), action,
                            KeyModifiers::None, // SDL mouse events carry no modifier state; track separately if needed
                            event.button.x, event.button.y));
                    break;
                }

                case SDL_EVENT_MOUSE_WHEEL:
                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<MouseScrollEvent>(event.wheel.x, event.wheel.y));
                    break;

                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                    const ButtonAction action =
                            (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) ? ButtonAction::Press : ButtonAction::Release;
                    m_windowEventBus->dispatch_immediate(std::make_shared<GamepadButtonEvent>(
                            static_cast<int>(event.gbutton.which),
                            sdl_gamepad_button_to_ribble(static_cast<SDL_GamepadButton>(event.gbutton.button)),
                            action));
                    break;
                }

                case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                    // SDL axis range: [-32768, 32767] → normalize to [-1, 1]
                    const float raw = event.gaxis.value / 32767.f;
                    const float normalized = std::clamp(raw, -1.f, 1.f);
                    m_windowEventBus->dispatch_immediate(std::make_shared<GamepadAxisEvent>(
                            static_cast<int>(event.gaxis.which),
                            sdl_gamepad_axis_to_ribble(static_cast<SDL_GamepadAxis>(event.gaxis.axis)), normalized));
                    break;
                }

                case SDL_EVENT_GAMEPAD_ADDED: {
                    SDL_Gamepad *pad = SDL_OpenGamepad(event.gdevice.which);
                    const char *name = pad ? SDL_GetGamepadName(pad) : "Unknown";
                    m_windowEventBus->dispatch_immediate(std::make_shared<GamepadConnectedEvent>(
                            static_cast<int>(event.gdevice.which), name ? name : "Unknown"));
                    break;
                }

                case SDL_EVENT_GAMEPAD_REMOVED:
                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<GamepadDisconnectedEvent>(static_cast<int>(event.gdevice.which)));
                    break;

                case SDL_EVENT_DROP_FILE: {
                    std::vector<std::string> paths;
                    if (event.drop.data)
                        paths.emplace_back(event.drop.data);
                    m_windowEventBus->dispatch_immediate(
                            std::make_shared<DropEvent>(std::move(paths), event.drop.x, event.drop.y));
                    break;
                }

                default:
                    break;
            }
        }

        return Ok();
    }

    Result<void, WindowBackend::Failure> SDLWindow::shutdown() {
        WindowBackend::shutdown();
        if (!m_window) {
            return Fail(
                    RIBBLE_ERROR(Failure::ShutdownFailure, "Failed to shut down SDL3 window cause it doesnt exist."));
        }
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return Ok();
    }

    void *SDLWindow::native_handle() const {
        // Return the SDL_Window* directly for OpenGL context creation
        // This allows the OpenGL backend to create a context from this window
        return static_cast<void *>(m_window);
    }

} // namespace backend
