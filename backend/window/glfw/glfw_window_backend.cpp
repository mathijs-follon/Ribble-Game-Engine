#include "glfw_window_backend.h"

#include <ribble/core/logger.h>
#include "../../common/window_events.h"

using namespace ribble::core;

namespace backend {

    static KeyboardKey glfw_key_to_ribble(int key) {
        switch (key) {
            case GLFW_KEY_SPACE:
                return KeyboardKey::Space;
            case GLFW_KEY_APOSTROPHE:
                return KeyboardKey::Apostrophe;
            case GLFW_KEY_COMMA:
                return KeyboardKey::Comma;
            case GLFW_KEY_MINUS:
                return KeyboardKey::Minus;
            case GLFW_KEY_PERIOD:
                return KeyboardKey::Period;
            case GLFW_KEY_SLASH:
                return KeyboardKey::Slash;
            case GLFW_KEY_0:
                return KeyboardKey::Num0;
            case GLFW_KEY_1:
                return KeyboardKey::Num1;
            case GLFW_KEY_2:
                return KeyboardKey::Num2;
            case GLFW_KEY_3:
                return KeyboardKey::Num3;
            case GLFW_KEY_4:
                return KeyboardKey::Num4;
            case GLFW_KEY_5:
                return KeyboardKey::Num5;
            case GLFW_KEY_6:
                return KeyboardKey::Num6;
            case GLFW_KEY_7:
                return KeyboardKey::Num7;
            case GLFW_KEY_8:
                return KeyboardKey::Num8;
            case GLFW_KEY_9:
                return KeyboardKey::Num9;
            case GLFW_KEY_SEMICOLON:
                return KeyboardKey::Semicolon;
            case GLFW_KEY_EQUAL:
                return KeyboardKey::Equal;
            case GLFW_KEY_A:
                return KeyboardKey::A;
            case GLFW_KEY_B:
                return KeyboardKey::B;
            case GLFW_KEY_C:
                return KeyboardKey::C;
            case GLFW_KEY_D:
                return KeyboardKey::D;
            case GLFW_KEY_E:
                return KeyboardKey::E;
            case GLFW_KEY_F:
                return KeyboardKey::F;
            case GLFW_KEY_G:
                return KeyboardKey::G;
            case GLFW_KEY_H:
                return KeyboardKey::H;
            case GLFW_KEY_I:
                return KeyboardKey::I;
            case GLFW_KEY_J:
                return KeyboardKey::J;
            case GLFW_KEY_K:
                return KeyboardKey::K;
            case GLFW_KEY_L:
                return KeyboardKey::L;
            case GLFW_KEY_M:
                return KeyboardKey::M;
            case GLFW_KEY_N:
                return KeyboardKey::N;
            case GLFW_KEY_O:
                return KeyboardKey::O;
            case GLFW_KEY_P:
                return KeyboardKey::P;
            case GLFW_KEY_Q:
                return KeyboardKey::Q;
            case GLFW_KEY_R:
                return KeyboardKey::R;
            case GLFW_KEY_S:
                return KeyboardKey::S;
            case GLFW_KEY_T:
                return KeyboardKey::T;
            case GLFW_KEY_U:
                return KeyboardKey::U;
            case GLFW_KEY_V:
                return KeyboardKey::V;
            case GLFW_KEY_W:
                return KeyboardKey::W;
            case GLFW_KEY_X:
                return KeyboardKey::X;
            case GLFW_KEY_Y:
                return KeyboardKey::Y;
            case GLFW_KEY_Z:
                return KeyboardKey::Z;
            case GLFW_KEY_LEFT_BRACKET:
                return KeyboardKey::LeftBracket;
            case GLFW_KEY_BACKSLASH:
                return KeyboardKey::Backslash;
            case GLFW_KEY_RIGHT_BRACKET:
                return KeyboardKey::RightBracket;
            case GLFW_KEY_GRAVE_ACCENT:
                return KeyboardKey::GraveAccent;
            case GLFW_KEY_ESCAPE:
                return KeyboardKey::Escape;
            case GLFW_KEY_ENTER:
                return KeyboardKey::Enter;
            case GLFW_KEY_TAB:
                return KeyboardKey::Tab;
            case GLFW_KEY_BACKSPACE:
                return KeyboardKey::Backspace;
            case GLFW_KEY_INSERT:
                return KeyboardKey::Insert;
            case GLFW_KEY_DELETE:
                return KeyboardKey::Delete;
            case GLFW_KEY_RIGHT:
                return KeyboardKey::Right;
            case GLFW_KEY_LEFT:
                return KeyboardKey::Left;
            case GLFW_KEY_DOWN:
                return KeyboardKey::Down;
            case GLFW_KEY_UP:
                return KeyboardKey::Up;
            case GLFW_KEY_PAGE_UP:
                return KeyboardKey::PageUp;
            case GLFW_KEY_PAGE_DOWN:
                return KeyboardKey::PageDown;
            case GLFW_KEY_HOME:
                return KeyboardKey::Home;
            case GLFW_KEY_END:
                return KeyboardKey::End;
            case GLFW_KEY_CAPS_LOCK:
                return KeyboardKey::CapsLock;
            case GLFW_KEY_SCROLL_LOCK:
                return KeyboardKey::ScrollLock;
            case GLFW_KEY_NUM_LOCK:
                return KeyboardKey::NumLock;
            case GLFW_KEY_PRINT_SCREEN:
                return KeyboardKey::PrintScreen;
            case GLFW_KEY_PAUSE:
                return KeyboardKey::Pause;
            case GLFW_KEY_F1:
                return KeyboardKey::F1;
            case GLFW_KEY_F2:
                return KeyboardKey::F2;
            case GLFW_KEY_F3:
                return KeyboardKey::F3;
            case GLFW_KEY_F4:
                return KeyboardKey::F4;
            case GLFW_KEY_F5:
                return KeyboardKey::F5;
            case GLFW_KEY_F6:
                return KeyboardKey::F6;
            case GLFW_KEY_F7:
                return KeyboardKey::F7;
            case GLFW_KEY_F8:
                return KeyboardKey::F8;
            case GLFW_KEY_F9:
                return KeyboardKey::F9;
            case GLFW_KEY_F10:
                return KeyboardKey::F10;
            case GLFW_KEY_F11:
                return KeyboardKey::F11;
            case GLFW_KEY_F12:
                return KeyboardKey::F12;
            case GLFW_KEY_LEFT_SHIFT:
                return KeyboardKey::LeftShift;
            case GLFW_KEY_LEFT_CONTROL:
                return KeyboardKey::LeftControl;
            case GLFW_KEY_LEFT_ALT:
                return KeyboardKey::LeftAlt;
            case GLFW_KEY_LEFT_SUPER:
                return KeyboardKey::LeftSuper;
            case GLFW_KEY_RIGHT_SHIFT:
                return KeyboardKey::RightShift;
            case GLFW_KEY_RIGHT_CONTROL:
                return KeyboardKey::RightControl;
            case GLFW_KEY_RIGHT_ALT:
                return KeyboardKey::RightAlt;
            case GLFW_KEY_RIGHT_SUPER:
                return KeyboardKey::RightSuper;
            case GLFW_KEY_MENU:
                return KeyboardKey::Menu;
            default:
                return KeyboardKey::Unknown;
        }
    }

    static MouseButton glfw_mouse_button_to_ribble(int button) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_RIGHT:
                return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case GLFW_MOUSE_BUTTON_4:
                return MouseButton::Button4;
            case GLFW_MOUSE_BUTTON_5:
                return MouseButton::Button5;
            default:
                return MouseButton::Unknown;
        }
    }

    static KeyModifiers glfw_mods_to_ribble(int mods) {
        KeyModifiers out = KeyModifiers::None;
        if (mods & GLFW_MOD_SHIFT)
            out = out | KeyModifiers::Shift;
        if (mods & GLFW_MOD_CONTROL)
            out = out | KeyModifiers::Control;
        if (mods & GLFW_MOD_ALT)
            out = out | KeyModifiers::Alt;
        if (mods & GLFW_MOD_SUPER)
            out = out | KeyModifiers::Super;
        if (mods & GLFW_MOD_CAPS_LOCK)
            out = out | KeyModifiers::CapsLock;
        if (mods & GLFW_MOD_NUM_LOCK)
            out = out | KeyModifiers::NumLock;
        return out;
    }

    Result<void, WindowBackend::Failure> GLFWWindow::initialize(int width, int height, const char *title) {
        WindowBackend::initialize(width, height, title);

        // Initialize GLFW if not already initialized
        if (!m_glfwInitialized) {
            if (!glfwInit()) {
                return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "glfwInit failed"));
            }
            m_glfwInitialized = true;
            RIBBLE_LOG_INFO("GLFW initialized.");
        }

        // Set OpenGL context hints BEFORE creating the window
        // This is critical - hints must be set before window creation
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
#if defined(RIBBLE_DEBUG)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        // Create window with OpenGL context
        m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!m_window) {
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "glfwCreateWindow failed"));
        }
        RIBBLE_LOG_INFO("GLFW window created: {}x{}", width, height);

        // Set window user pointer for callbacks
        glfwSetWindowUserPointer(m_window, this);

        // Set up callbacks
        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_shouldClose = true;
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
        });

        // Use framebuffer size callback for accurate pixel dimensions (HiDPI support)
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *window, int w, int h) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<WindowResizeEvent>(w, h));
        });

        glfwSetWindowPosCallback(m_window, [](GLFWwindow *window, int x, int y) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<WindowMoveEvent>(x, y));
        });

        glfwSetWindowFocusCallback(m_window, [](GLFWwindow *window, int focused) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<WindowFocusEvent>(focused == GLFW_TRUE));
        });

        glfwSetWindowIconifyCallback(m_window, [](GLFWwindow *window, int iconified) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(
                    std::make_shared<WindowMinimizeEvent>(iconified == GLFW_TRUE));
        });

        glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow *window, int maximized) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(
                    std::make_shared<WindowMaximizeEvent>(maximized == GLFW_TRUE));
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            KeyboardKey ribbleKey = glfw_key_to_ribble(key);
            KeyModifiers ribbleMods = glfw_mods_to_ribble(mods);

            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                backend->m_windowEventBus->dispatch_immediate(
                        std::make_shared<KeyDownEvent>(ribbleKey, ribbleMods, scancode, action == GLFW_REPEAT));
            } else if (action == GLFW_RELEASE) {
                backend->m_windowEventBus->dispatch_immediate(
                        std::make_shared<KeyUpEvent>(ribbleKey, ribbleMods, scancode));
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            MouseButton ribbleButton = glfw_mouse_button_to_ribble(button);
            KeyModifiers ribbleMods = glfw_mods_to_ribble(mods);

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            ButtonAction buttonAction = (action == GLFW_PRESS) ? ButtonAction::Press : ButtonAction::Release;
            backend->m_windowEventBus->dispatch_immediate(
                    std::make_shared<MouseButtonEvent>(ribbleButton, buttonAction, ribbleMods, x, y));
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            double dx = 0.0, dy = 0.0;
            if (backend->m_firstMouseMove) {
                backend->m_firstMouseMove = false;
            } else {
                dx = x - backend->m_lastMouseX;
                dy = y - backend->m_lastMouseY;
            }
            backend->m_lastMouseX = x;
            backend->m_lastMouseY = y;
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<MouseMoveEvent>(x, y, dx, dy));
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xoffset, double yoffset) {
            auto *backend = static_cast<GLFWWindow *>(glfwGetWindowUserPointer(window));
            backend->m_windowEventBus->dispatch_immediate(std::make_shared<MouseScrollEvent>(xoffset, yoffset));
        });

        return Ok();
    }

    Result<void, WindowBackend::Failure> GLFWWindow::poll_events() {
        glfwPollEvents();

        // Check if window should close (in case it was closed externally)
        if (m_window && glfwWindowShouldClose(m_window) && !m_shouldClose) {
            m_shouldClose = true;
            m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
        }

        return Ok();
    }

    Result<void, WindowBackend::Failure> GLFWWindow::shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        if (m_glfwInitialized) {
            glfwTerminate();
            m_glfwInitialized = false;
        }

        return Ok();
    }

    void *GLFWWindow::native_handle() const { return static_cast<void *>(m_window); }

} // namespace backend
