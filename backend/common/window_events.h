#pragma once
#include "ribble/core/event.h"

namespace backend {

    enum class KeyboardKey {
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Semicolon,
        Equal,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,

        Kp0,
        Kp1,
        Kp2,
        Kp3,
        Kp4,
        Kp5,
        Kp6,
        Kp7,
        Kp8,
        Kp9,
        KpDecimal,
        KpDivide,
        KpMultiply,
        KpSubtract,
        KpAdd,
        KpEnter,
        KpEqual,

        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper,
        RightShift,
        RightControl,
        RightAlt,
        RightSuper,
        Menu,

        Unknown,
    };

    enum class KeyModifiers : uint32_t {
        None = 0,
        Shift = 1 << 0,
        Control = 1 << 1,
        Alt = 1 << 2,
        Super = 1 << 3,
        CapsLock = 1 << 4,
        NumLock = 1 << 5,
    };

    inline KeyModifiers operator|(KeyModifiers a, KeyModifiers b) {
        return static_cast<KeyModifiers>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline bool operator&(KeyModifiers a, KeyModifiers b) {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    enum class MouseButton {
        Unknown = 0,
        Left,
        Right,
        Middle,
        Button4,
        Button5,
    };

    enum class GamepadButton {
        Unknown = 0,
        South, // A / Cross
        East, // B / Circle
        West, // X / Square
        North, // Y / Triangle
        LeftBumper,
        RightBumper,
        LeftTriggerButton,
        RightTriggerButton,
        Select,
        Start,
        Guide,
        LeftThumb,
        RightThumb,
        DpadUp,
        DpadDown,
        DpadLeft,
        DpadRight,
    };

    enum class GamepadAxis {
        Unknown = 0,
        LeftX,
        LeftY,
        RightX,
        RightY,
        LeftTrigger, // [-1, 1] or [0, 1] depending on backend; normalize to [-1, 1]
        RightTrigger,
    };

    namespace detail {
        inline size_t next_event_type_id() {
            static size_t id = 0;
            return ++id;
        }

        template<typename T>
        inline size_t event_type_id() {
            static size_t id = next_event_type_id();
            return id;
        }
    } // namespace detail

    template<typename Derived>
    class WindowEvent : public ribble::core::Event {
    public:
        [[nodiscard]] size_t type_id() const override { return detail::event_type_id<Derived>(); }
        [[nodiscard]] std::string type_str() const override { return Derived::static_type_str(); }
    };

    class KeyDownEvent : public WindowEvent<KeyDownEvent> {
    public:
        KeyDownEvent() = default;
        KeyDownEvent(KeyboardKey key, KeyModifiers mods, int scancode, bool repeat) :
            m_key(key), m_mods(mods), m_scancode(scancode), m_repeat(repeat) {}

        [[nodiscard]] static std::string static_type_str() { return "KeyDownEvent"; }

        [[nodiscard]] KeyboardKey key() const { return m_key; }
        [[nodiscard]] KeyModifiers mods() const { return m_mods; }
        [[nodiscard]] int scancode() const { return m_scancode; }
        [[nodiscard]] bool is_repeat() const { return m_repeat; }

        [[nodiscard]] bool has_mod(KeyModifiers mod) const { return m_mods & mod; }

    private:
        KeyboardKey m_key{KeyboardKey::Unknown};
        KeyModifiers m_mods{KeyModifiers::None};
        int m_scancode{0};
        bool m_repeat{false};
    };

    class KeyUpEvent : public WindowEvent<KeyUpEvent> {
    public:
        KeyUpEvent() = default;
        KeyUpEvent(KeyboardKey key, KeyModifiers mods, int scancode) : m_key(key), m_mods(mods), m_scancode(scancode) {}

        [[nodiscard]] static std::string static_type_str() { return "KeyUpEvent"; }

        [[nodiscard]] KeyboardKey key() const { return m_key; }
        [[nodiscard]] KeyModifiers mods() const { return m_mods; }
        [[nodiscard]] int scancode() const { return m_scancode; }

        [[nodiscard]] bool has_mod(KeyModifiers mod) const { return m_mods & mod; }

    private:
        KeyboardKey m_key{KeyboardKey::Unknown};
        KeyModifiers m_mods{KeyModifiers::None};
        int m_scancode{0};
    };

    class TextInputEvent : public WindowEvent<TextInputEvent> {
    public:
        TextInputEvent() = default;
        explicit TextInputEvent(uint32_t codepoint) : m_codepoint(codepoint) {}

        [[nodiscard]] static std::string static_type_str() { return "TextInputEvent"; }

        /// Unicode code point (UTF-32).
        [[nodiscard]] uint32_t codepoint() const { return m_codepoint; }

    private:
        uint32_t m_codepoint{0};
    };

    class MouseMoveEvent : public WindowEvent<MouseMoveEvent> {
    public:
        MouseMoveEvent() = default;
        MouseMoveEvent(double x, double y, double dx, double dy) : m_x(x), m_y(y), m_dx(dx), m_dy(dy) {}

        [[nodiscard]] static std::string static_type_str() { return "MouseMoveEvent"; }

        /// Cursor position in window-space pixels.
        [[nodiscard]] double x() const { return m_x; }
        [[nodiscard]] double y() const { return m_y; }
        /// Delta from last position.
        [[nodiscard]] double dx() const { return m_dx; }
        [[nodiscard]] double dy() const { return m_dy; }

    private:
        double m_x{0}, m_y{0};
        double m_dx{0}, m_dy{0};
    };

    enum class ButtonAction { Press, Release };

    class MouseButtonEvent : public WindowEvent<MouseButtonEvent> {
    public:
        MouseButtonEvent() = default;
        MouseButtonEvent(MouseButton button, ButtonAction action, KeyModifiers mods, double x, double y) :
            m_button(button), m_action(action), m_mods(mods), m_x(x), m_y(y) {}

        [[nodiscard]] static std::string static_type_str() { return "MouseButtonEvent"; }

        [[nodiscard]] MouseButton button() const { return m_button; }
        [[nodiscard]] ButtonAction action() const { return m_action; }
        [[nodiscard]] KeyModifiers mods() const { return m_mods; }
        [[nodiscard]] double x() const { return m_x; }
        [[nodiscard]] double y() const { return m_y; }

        [[nodiscard]] bool is_pressed() const { return m_action == ButtonAction::Press; }
        [[nodiscard]] bool is_released() const { return m_action == ButtonAction::Release; }

    private:
        MouseButton m_button{MouseButton::Unknown};
        ButtonAction m_action{ButtonAction::Press};
        KeyModifiers m_mods{KeyModifiers::None};
        double m_x{0}, m_y{0};
    };

    class MouseScrollEvent : public WindowEvent<MouseScrollEvent> {
    public:
        MouseScrollEvent() = default;
        MouseScrollEvent(double xoffset, double yoffset) : m_xoffset(xoffset), m_yoffset(yoffset) {}

        [[nodiscard]] static std::string static_type_str() { return "MouseScrollEvent"; }

        [[nodiscard]] double xoffset() const { return m_xoffset; }
        [[nodiscard]] double yoffset() const { return m_yoffset; }

    private:
        double m_xoffset{0}, m_yoffset{0};
    };

    class GamepadButtonEvent : public WindowEvent<GamepadButtonEvent> {
    public:
        GamepadButtonEvent() = default;
        GamepadButtonEvent(int gamepadId, GamepadButton button, ButtonAction action) :
            m_gamepadId(gamepadId), m_button(button), m_action(action) {}

        [[nodiscard]] static std::string static_type_str() { return "GamepadButtonEvent"; }

        [[nodiscard]] int gamepad_id() const { return m_gamepadId; }
        [[nodiscard]] GamepadButton button() const { return m_button; }
        [[nodiscard]] ButtonAction action() const { return m_action; }

        [[nodiscard]] bool is_pressed() const { return m_action == ButtonAction::Press; }
        [[nodiscard]] bool is_released() const { return m_action == ButtonAction::Release; }

    private:
        int m_gamepadId{0};
        GamepadButton m_button{GamepadButton::Unknown};
        ButtonAction m_action{ButtonAction::Press};
    };

    class GamepadAxisEvent : public WindowEvent<GamepadAxisEvent> {
    public:
        GamepadAxisEvent() = default;
        GamepadAxisEvent(int gamepadId, GamepadAxis axis, float value) :
            m_gamepadId(gamepadId), m_axis(axis), m_value(value) {}

        [[nodiscard]] static std::string static_type_str() { return "GamepadAxisEvent"; }

        [[nodiscard]] int gamepad_id() const { return m_gamepadId; }
        [[nodiscard]] GamepadAxis axis() const { return m_axis; }
        /// Normalized value in [-1.0, 1.0].
        [[nodiscard]] float value() const { return m_value; }

    private:
        int m_gamepadId{0};
        GamepadAxis m_axis{GamepadAxis::Unknown};
        float m_value{0.f};
    };

    class GamepadConnectedEvent : public WindowEvent<GamepadConnectedEvent> {
    public:
        GamepadConnectedEvent() = default;
        explicit GamepadConnectedEvent(int gamepadId, std::string name) :
            m_gamepadId(gamepadId), m_name(std::move(name)) {}

        [[nodiscard]] static std::string static_type_str() { return "GamepadConnectedEvent"; }

        [[nodiscard]] int gamepad_id() const { return m_gamepadId; }
        [[nodiscard]] const std::string &name() const { return m_name; }

    private:
        int m_gamepadId{0};
        std::string m_name;
    };

    class GamepadDisconnectedEvent : public WindowEvent<GamepadDisconnectedEvent> {
    public:
        GamepadDisconnectedEvent() = default;
        explicit GamepadDisconnectedEvent(int gamepadId) : m_gamepadId(gamepadId) {}

        [[nodiscard]] static std::string static_type_str() { return "GamepadDisconnectedEvent"; }

        [[nodiscard]] int gamepad_id() const { return m_gamepadId; }

    private:
        int m_gamepadId{0};
    };

    class WindowResizeEvent : public WindowEvent<WindowResizeEvent> {
    public:
        WindowResizeEvent() = default;
        WindowResizeEvent(int width, int height) : m_width(width), m_height(height) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowResizeEvent"; }

        [[nodiscard]] int width() const { return m_width; }
        [[nodiscard]] int height() const { return m_height; }

    private:
        int m_width{0}, m_height{0};
    };

    class WindowCloseEvent : public WindowEvent<WindowCloseEvent> {
    public:
        [[nodiscard]] static std::string static_type_str() { return "WindowCloseEvent"; }
    };

    class WindowFocusEvent : public WindowEvent<WindowFocusEvent> {
    public:
        WindowFocusEvent() = default;
        explicit WindowFocusEvent(bool focused) : m_focused(focused) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowFocusEvent"; }

        [[nodiscard]] bool focused() const { return m_focused; }

    private:
        bool m_focused{false};
    };

    class WindowMinimizeEvent : public WindowEvent<WindowMinimizeEvent> {
    public:
        WindowMinimizeEvent() = default;
        explicit WindowMinimizeEvent(bool minimized) : m_minimized(minimized) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowMinimizeEvent"; }

        [[nodiscard]] bool minimized() const { return m_minimized; }

    private:
        bool m_minimized{false};
    };

    class WindowMaximizeEvent : public WindowEvent<WindowMaximizeEvent> {
    public:
        WindowMaximizeEvent() = default;
        explicit WindowMaximizeEvent(bool maximized) : m_maximized(maximized) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowMaximizeEvent"; }

        [[nodiscard]] bool maximized() const { return m_maximized; }

    private:
        bool m_maximized{false};
    };

    class WindowMoveEvent : public WindowEvent<WindowMoveEvent> {
    public:
        WindowMoveEvent() = default;
        WindowMoveEvent(int x, int y) : m_x(x), m_y(y) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowMoveEvent"; }

        [[nodiscard]] int x() const { return m_x; }
        [[nodiscard]] int y() const { return m_y; }

    private:
        int m_x{0}, m_y{0};
    };

    /// Fired when the OS DPI / content-scale changes (e.g. dragging to a HiDPI monitor).
    class WindowContentScaleEvent : public WindowEvent<WindowContentScaleEvent> {
    public:
        WindowContentScaleEvent() = default;
        WindowContentScaleEvent(float xscale, float yscale) : m_xscale(xscale), m_yscale(yscale) {}

        [[nodiscard]] static std::string static_type_str() { return "WindowContentScaleEvent"; }

        [[nodiscard]] float xscale() const { return m_xscale; }
        [[nodiscard]] float yscale() const { return m_yscale; }

    private:
        float m_xscale{1.f}, m_yscale{1.f};
    };

    class DropEvent : public WindowEvent<DropEvent> {
    public:
        DropEvent() = default;
        explicit DropEvent(std::vector<std::string> paths, double x = 0, double y = 0) :
            m_paths(std::move(paths)), m_x(x), m_y(y) {}

        [[nodiscard]] static std::string static_type_str() { return "DropEvent"; }

        [[nodiscard]] const std::vector<std::string> &paths() const { return m_paths; }
        [[nodiscard]] double x() const { return m_x; }
        [[nodiscard]] double y() const { return m_y; }

    private:
        std::vector<std::string> m_paths;
        double m_x{0}, m_y{0};
    };

} // namespace backend
