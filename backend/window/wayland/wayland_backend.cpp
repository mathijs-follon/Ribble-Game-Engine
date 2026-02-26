// Include our event types before any lib that might define conflicting macros
#include "../../common/window_events.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-client.h>
#include <ribble/core/logger.h>

#include "xdg-shell-client-protocol.h"
#include "wayland_backend.h"

using namespace ribble::core;

namespace backend {

    namespace {

        static KeyboardKey xkb_keysym_to_ribble(xkb_keysym_t keysym) {
            switch (keysym) {
                case XKB_KEY_space:
                    return KeyboardKey::Space;
                case XKB_KEY_apostrophe:
                    return KeyboardKey::Apostrophe;
                case XKB_KEY_comma:
                    return KeyboardKey::Comma;
                case XKB_KEY_minus:
                    return KeyboardKey::Minus;
                case XKB_KEY_period:
                    return KeyboardKey::Period;
                case XKB_KEY_slash:
                    return KeyboardKey::Slash;
                case XKB_KEY_0:
                    return KeyboardKey::Num0;
                case XKB_KEY_1:
                    return KeyboardKey::Num1;
                case XKB_KEY_2:
                    return KeyboardKey::Num2;
                case XKB_KEY_3:
                    return KeyboardKey::Num3;
                case XKB_KEY_4:
                    return KeyboardKey::Num4;
                case XKB_KEY_5:
                    return KeyboardKey::Num5;
                case XKB_KEY_6:
                    return KeyboardKey::Num6;
                case XKB_KEY_7:
                    return KeyboardKey::Num7;
                case XKB_KEY_8:
                    return KeyboardKey::Num8;
                case XKB_KEY_9:
                    return KeyboardKey::Num9;
                case XKB_KEY_semicolon:
                    return KeyboardKey::Semicolon;
                case XKB_KEY_equal:
                    return KeyboardKey::Equal;
                case XKB_KEY_a:
                case XKB_KEY_A:
                    return KeyboardKey::A;
                case XKB_KEY_b:
                case XKB_KEY_B:
                    return KeyboardKey::B;
                case XKB_KEY_c:
                case XKB_KEY_C:
                    return KeyboardKey::C;
                case XKB_KEY_d:
                case XKB_KEY_D:
                    return KeyboardKey::D;
                case XKB_KEY_e:
                case XKB_KEY_E:
                    return KeyboardKey::E;
                case XKB_KEY_f:
                case XKB_KEY_F:
                    return KeyboardKey::F;
                case XKB_KEY_g:
                case XKB_KEY_G:
                    return KeyboardKey::G;
                case XKB_KEY_h:
                case XKB_KEY_H:
                    return KeyboardKey::H;
                case XKB_KEY_i:
                case XKB_KEY_I:
                    return KeyboardKey::I;
                case XKB_KEY_j:
                case XKB_KEY_J:
                    return KeyboardKey::J;
                case XKB_KEY_k:
                case XKB_KEY_K:
                    return KeyboardKey::K;
                case XKB_KEY_l:
                case XKB_KEY_L:
                    return KeyboardKey::L;
                case XKB_KEY_m:
                case XKB_KEY_M:
                    return KeyboardKey::M;
                case XKB_KEY_n:
                case XKB_KEY_N:
                    return KeyboardKey::N;
                case XKB_KEY_o:
                case XKB_KEY_O:
                    return KeyboardKey::O;
                case XKB_KEY_p:
                case XKB_KEY_P:
                    return KeyboardKey::P;
                case XKB_KEY_q:
                case XKB_KEY_Q:
                    return KeyboardKey::Q;
                case XKB_KEY_r:
                case XKB_KEY_R:
                    return KeyboardKey::R;
                case XKB_KEY_s:
                case XKB_KEY_S:
                    return KeyboardKey::S;
                case XKB_KEY_t:
                case XKB_KEY_T:
                    return KeyboardKey::T;
                case XKB_KEY_u:
                case XKB_KEY_U:
                    return KeyboardKey::U;
                case XKB_KEY_v:
                case XKB_KEY_V:
                    return KeyboardKey::V;
                case XKB_KEY_w:
                case XKB_KEY_W:
                    return KeyboardKey::W;
                case XKB_KEY_x:
                case XKB_KEY_X:
                    return KeyboardKey::X;
                case XKB_KEY_y:
                case XKB_KEY_Y:
                    return KeyboardKey::Y;
                case XKB_KEY_z:
                case XKB_KEY_Z:
                    return KeyboardKey::Z;
                case XKB_KEY_bracketleft:
                    return KeyboardKey::LeftBracket;
                case XKB_KEY_backslash:
                    return KeyboardKey::Backslash;
                case XKB_KEY_bracketright:
                    return KeyboardKey::RightBracket;
                case XKB_KEY_grave:
                    return KeyboardKey::GraveAccent;
                case XKB_KEY_F1:
                    return KeyboardKey::F1;
                case XKB_KEY_F2:
                    return KeyboardKey::F2;
                case XKB_KEY_F3:
                    return KeyboardKey::F3;
                case XKB_KEY_F4:
                    return KeyboardKey::F4;
                case XKB_KEY_F5:
                    return KeyboardKey::F5;
                case XKB_KEY_F6:
                    return KeyboardKey::F6;
                case XKB_KEY_F7:
                    return KeyboardKey::F7;
                case XKB_KEY_F8:
                    return KeyboardKey::F8;
                case XKB_KEY_F9:
                    return KeyboardKey::F9;
                case XKB_KEY_F10:
                    return KeyboardKey::F10;
                case XKB_KEY_F11:
                    return KeyboardKey::F11;
                case XKB_KEY_F12:
                    return KeyboardKey::F12;
                case XKB_KEY_Escape:
                    return KeyboardKey::Escape;
                case XKB_KEY_Return:
                case XKB_KEY_KP_Enter:
                    return KeyboardKey::Enter;
                case XKB_KEY_Tab:
                    return KeyboardKey::Tab;
                case XKB_KEY_BackSpace:
                    return KeyboardKey::Backspace;
                case XKB_KEY_Insert:
                    return KeyboardKey::Insert;
                case XKB_KEY_Delete:
                    return KeyboardKey::Delete;
                case XKB_KEY_Right:
                    return KeyboardKey::Right;
                case XKB_KEY_Left:
                    return KeyboardKey::Left;
                case XKB_KEY_Down:
                    return KeyboardKey::Down;
                case XKB_KEY_Up:
                    return KeyboardKey::Up;
                case XKB_KEY_Page_Up:
                    return KeyboardKey::PageUp;
                case XKB_KEY_Page_Down:
                    return KeyboardKey::PageDown;
                case XKB_KEY_Home:
                    return KeyboardKey::Home;
                case XKB_KEY_End:
                    return KeyboardKey::End;
                case XKB_KEY_Caps_Lock:
                    return KeyboardKey::CapsLock;
                case XKB_KEY_Num_Lock:
                    return KeyboardKey::NumLock;
                case XKB_KEY_Print:
                    return KeyboardKey::PrintScreen;
                case XKB_KEY_Pause:
                    return KeyboardKey::Pause;
                case XKB_KEY_KP_0:
                    return KeyboardKey::Kp0;
                case XKB_KEY_KP_1:
                    return KeyboardKey::Kp1;
                case XKB_KEY_KP_2:
                    return KeyboardKey::Kp2;
                case XKB_KEY_KP_3:
                    return KeyboardKey::Kp3;
                case XKB_KEY_KP_4:
                    return KeyboardKey::Kp4;
                case XKB_KEY_KP_5:
                    return KeyboardKey::Kp5;
                case XKB_KEY_KP_6:
                    return KeyboardKey::Kp6;
                case XKB_KEY_KP_7:
                    return KeyboardKey::Kp7;
                case XKB_KEY_KP_8:
                    return KeyboardKey::Kp8;
                case XKB_KEY_KP_9:
                    return KeyboardKey::Kp9;
                case XKB_KEY_KP_Decimal:
                    return KeyboardKey::KpDecimal;
                case XKB_KEY_KP_Divide:
                    return KeyboardKey::KpDivide;
                case XKB_KEY_KP_Multiply:
                    return KeyboardKey::KpMultiply;
                case XKB_KEY_KP_Subtract:
                    return KeyboardKey::KpSubtract;
                case XKB_KEY_KP_Add:
                    return KeyboardKey::KpAdd;
                case XKB_KEY_KP_Equal:
                    return KeyboardKey::KpEqual;
                case XKB_KEY_Shift_L:
                    return KeyboardKey::LeftShift;
                case XKB_KEY_Control_L:
                    return KeyboardKey::LeftControl;
                case XKB_KEY_Alt_L:
                    return KeyboardKey::LeftAlt;
                case XKB_KEY_Super_L:
                    return KeyboardKey::LeftSuper;
                case XKB_KEY_Shift_R:
                    return KeyboardKey::RightShift;
                case XKB_KEY_Control_R:
                    return KeyboardKey::RightControl;
                case XKB_KEY_Alt_R:
                    return KeyboardKey::RightAlt;
                case XKB_KEY_Super_R:
                    return KeyboardKey::RightSuper;
                case XKB_KEY_Menu:
                    return KeyboardKey::Menu;
                default:
                    return KeyboardKey::Unknown;
            }
        }

        static MouseButton evdev_button_to_ribble(uint32_t button) {
            switch (button) {
                case 272:
                    return MouseButton::Left;
                case 274:
                    return MouseButton::Right;
                case 273:
                    return MouseButton::Middle;
                case 275:
                    return MouseButton::Button4;
                case 276:
                    return MouseButton::Button5;
                default:
                    return MouseButton::Unknown;
            }
        }

        static KeyModifiers xkb_mods_to_ribble(uint32_t mods) {
            KeyModifiers out = KeyModifiers::None;
            if (mods & (1 << 0))
                out = out | KeyModifiers::Shift;
            if (mods & (1 << 1))
                out = out | KeyModifiers::Control;
            if (mods & (1 << 2))
                out = out | KeyModifiers::Alt;
            if (mods & (1 << 3))
                out = out | KeyModifiers::Super;
            if (mods & (1 << 4))
                out = out | KeyModifiers::CapsLock;
            if (mods & (1 << 5))
                out = out | KeyModifiers::NumLock;
            return out;
        }

    } // namespace

    WaylandBackend::WaylandBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus)
        : WindowBackend(std::move(windowEventBus)) {}

    WaylandBackend::~WaylandBackend() { WaylandBackend::shutdown(); }

    Result<void, WindowBackend::Failure> WaylandBackend::initialize(int width, int height, const char *title) {
        WindowBackend::initialize(width, height, title);
        m_width = width;
        m_height = height;
        m_title = title;

        m_display = wl_display_connect(nullptr);
        if (!m_display) {
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "wl_display_connect failed"));
        }

        m_registry = wl_display_get_registry(m_display);
        if (!m_registry) {
            wl_display_disconnect(m_display);
            m_display = nullptr;
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "wl_display_get_registry failed"));
        }

        static const wl_registry_listener registry_listener = {global_registry_handler, global_registry_remover};
        wl_registry_add_listener(m_registry, &registry_listener, this);
        wl_display_roundtrip(m_display);

        if (!m_compositor || !m_xdg_wm_base) {
            shutdown();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure,
                                    "Compositor or xdg_wm_base not found"));
        }

        m_surface = wl_compositor_create_surface(m_compositor);
        if (!m_surface) {
            shutdown();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "wl_compositor_create_surface failed"));
        }

        m_xdg_surface = xdg_wm_base_get_xdg_surface(m_xdg_wm_base, m_surface);
        if (!m_xdg_surface) {
            shutdown();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "xdg_wm_base_get_xdg_surface failed"));
        }

        static const xdg_surface_listener xdg_surface_listener = {xdg_surface_configure};
        xdg_surface_add_listener(m_xdg_surface, &xdg_surface_listener, this);

        m_xdg_toplevel = xdg_surface_get_toplevel(m_xdg_surface);
        if (!m_xdg_toplevel) {
            shutdown();
            return Fail(RIBBLE_ERROR(WindowBackend::Failure::InitializationFailure, "xdg_surface_get_toplevel failed"));
        }

        static const xdg_toplevel_listener xdg_toplevel_listener = {
            xdg_toplevel_configure, xdg_toplevel_close, nullptr, nullptr}; // configure_bounds, wm_capabilities
        xdg_toplevel_add_listener(m_xdg_toplevel, &xdg_toplevel_listener, this);

        xdg_toplevel_set_title(m_xdg_toplevel, title);
        xdg_toplevel_set_app_id(m_xdg_toplevel, "ribble");

        wl_surface_commit(m_surface);
        wl_display_roundtrip(m_display);

        setup_input();

        RIBBLE_LOG_INFO("Wayland window created: {}x{}", width, height);
        return Ok();
    }

    void WaylandBackend::setup_input() {
        if (!m_seat)
            return;
        m_pointer = wl_seat_get_pointer(m_seat);
        m_keyboard = wl_seat_get_keyboard(m_seat);

        m_xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        if (!m_xkb_context)
            return;

        if (m_pointer) {
            static const wl_pointer_listener pointer_listener = {
                wl_pointer_enter, wl_pointer_leave, wl_pointer_motion, wl_pointer_button,
                wl_pointer_axis,  nullptr, nullptr, nullptr, nullptr, nullptr};
            wl_pointer_add_listener(m_pointer, &pointer_listener, this);
        }

        if (m_keyboard) {
            static const wl_keyboard_listener keyboard_listener = {
                wl_keyboard_keymap, wl_keyboard_enter, wl_keyboard_leave,
                wl_keyboard_key,    wl_keyboard_modifiers};
            wl_keyboard_add_listener(m_keyboard, &keyboard_listener, this);
        }
    }

    void WaylandBackend::release_input() {
        if (m_xkb_state) {
            xkb_state_unref(m_xkb_state);
            m_xkb_state = nullptr;
        }
        if (m_xkb_keymap) {
            xkb_keymap_unref(m_xkb_keymap);
            m_xkb_keymap = nullptr;
        }
        if (m_xkb_context) {
            xkb_context_unref(m_xkb_context);
            m_xkb_context = nullptr;
        }
        if (m_pointer) {
            wl_pointer_destroy(m_pointer);
            m_pointer = nullptr;
        }
        if (m_keyboard) {
            wl_keyboard_destroy(m_keyboard);
            m_keyboard = nullptr;
        }
    }

    void WaylandBackend::global_registry_handler(void *data, wl_registry *registry, uint32_t id,
                                                const char *interface, uint32_t /*version*/) {
        auto *self = static_cast<WaylandBackend *>(data);
        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            self->m_compositor =
                static_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, 4));
        } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
            self->m_xdg_wm_base =
                static_cast<xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
        } else if (strcmp(interface, wl_seat_interface.name) == 0) {
            self->m_seat = static_cast<wl_seat *>(wl_registry_bind(registry, id, &wl_seat_interface, 5));
        }
    }

    void WaylandBackend::global_registry_remover(void * /*data*/, wl_registry * /*registry*/, uint32_t /*id*/) {}

    void WaylandBackend::xdg_wm_base_ping(void * /*data*/, xdg_wm_base *wm_base, uint32_t serial) {
        xdg_wm_base_pong(wm_base, serial);
    }

    void WaylandBackend::xdg_surface_configure(void *data, xdg_surface *surface, uint32_t serial) {
        auto *self = static_cast<WaylandBackend *>(data);
        xdg_surface_ack_configure(surface, serial);
        self->m_configured = true;
    }

    void WaylandBackend::xdg_toplevel_configure(void *data, xdg_toplevel * /*toplevel*/, int32_t width,
                                                int32_t height, struct wl_array * /*states*/) {
        auto *self = static_cast<WaylandBackend *>(data);
        if (width > 0 && height > 0) {
            self->m_width = width;
            self->m_height = height;
            self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowResizeEvent>(width, height));
        }
    }

    void WaylandBackend::xdg_toplevel_close(void *data, xdg_toplevel * /*toplevel*/) {
        auto *self = static_cast<WaylandBackend *>(data);
        self->m_closed = true;
        self->m_windowEventBus->dispatch_immediate(std::make_shared<WindowCloseEvent>());
    }

    void WaylandBackend::wl_pointer_enter(void * /*data*/, wl_pointer * /*pointer*/, uint32_t /*serial*/,
                                         wl_surface * /*surface*/, wl_fixed_t x, wl_fixed_t y) {
        // Focus gained - could dispatch WindowFocusEvent
    }

    void WaylandBackend::wl_pointer_leave(void * /*data*/, wl_pointer * /*pointer*/, uint32_t /*serial*/,
                                         wl_surface * /*surface*/) {}

    void WaylandBackend::wl_pointer_motion(void *data, wl_pointer * /*pointer*/, uint32_t /*time*/,
                                          wl_fixed_t x, wl_fixed_t y) {
        auto *self = static_cast<WaylandBackend *>(data);
        double mx = wl_fixed_to_double(x);
        double my = wl_fixed_to_double(y);
        double dx, dy;
        if (self->m_firstMotion) {
            self->m_firstMotion = false;
            dx = dy = 0;
        } else {
            dx = mx - self->m_lastMouseX;
            dy = my - self->m_lastMouseY;
        }
        self->m_lastMouseX = mx;
        self->m_lastMouseY = my;
        self->m_windowEventBus->dispatch_immediate(
            std::make_shared<MouseMoveEvent>(self->m_lastMouseX, self->m_lastMouseY, dx, dy));
    }

    void WaylandBackend::wl_pointer_button(void *data, wl_pointer * /*pointer*/, uint32_t /*serial*/,
                                          uint32_t /*time*/, uint32_t button, uint32_t state) {
        auto *self = static_cast<WaylandBackend *>(data);
        MouseButton mb = evdev_button_to_ribble(button);
        ButtonAction action = (state == WL_POINTER_BUTTON_STATE_PRESSED) ? ButtonAction::Press : ButtonAction::Release;
        self->m_windowEventBus->dispatch_immediate(
            std::make_shared<MouseButtonEvent>(mb, action, KeyModifiers::None, self->m_lastMouseX, self->m_lastMouseY));
    }

    void WaylandBackend::wl_pointer_axis(void *data, wl_pointer * /*pointer*/, uint32_t /*time*/,
                                        uint32_t axis, wl_fixed_t value) {
        auto *self = static_cast<WaylandBackend *>(data);
        double v = static_cast<double>(value) / 256.0;
        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
            self->m_windowEventBus->dispatch_immediate(std::make_shared<MouseScrollEvent>(0, -v));
        } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            self->m_windowEventBus->dispatch_immediate(std::make_shared<MouseScrollEvent>(-v, 0));
        }
    }

    void WaylandBackend::wl_keyboard_keymap(void *data, wl_keyboard * /*keyboard*/, uint32_t format, int fd,
                                           uint32_t size) {
        auto *self = static_cast<WaylandBackend *>(data);
        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            close(fd);
            return;
        }
        void *map_str = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (map_str == MAP_FAILED)
            return;
        if (self->m_xkb_keymap) {
            xkb_keymap_unref(self->m_xkb_keymap);
            self->m_xkb_keymap = nullptr;
        }
        if (self->m_xkb_state) {
            xkb_state_unref(self->m_xkb_state);
            self->m_xkb_state = nullptr;
        }
        self->m_xkb_keymap = xkb_keymap_new_from_string(self->m_xkb_context, static_cast<const char *>(map_str),
                                                        XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(map_str, size);
        if (self->m_xkb_keymap)
            self->m_xkb_state = xkb_state_new(self->m_xkb_keymap);
    }

    void WaylandBackend::wl_keyboard_enter(void * /*data*/, wl_keyboard * /*keyboard*/, uint32_t /*serial*/,
                                          wl_surface * /*surface*/, wl_array * /*keys*/) {
        // Could dispatch WindowFocusEvent(true)
    }

    void WaylandBackend::wl_keyboard_leave(void * /*data*/, wl_keyboard * /*keyboard*/, uint32_t /*serial*/,
                                          wl_surface * /*surface*/) {}

    void WaylandBackend::wl_keyboard_key(void *data, wl_keyboard * /*keyboard*/, uint32_t /*serial*/,
                                        uint32_t /*time*/, uint32_t key, uint32_t state) {
        auto *self = static_cast<WaylandBackend *>(data);
        int scancode = static_cast<int>(key + 8); // linux evdev offset
        xkb_keysym_t keysym = XKB_KEY_NoSymbol;
        if (self->m_xkb_state) {
            keysym = xkb_state_key_get_one_sym(self->m_xkb_state, key + 8);
        }
        KeyboardKey k = xkb_keysym_to_ribble(keysym);
        KeyModifiers mods = KeyModifiers::None;
        if (self->m_xkb_state) {
            mods = xkb_mods_to_ribble(xkb_state_serialize_mods(
                self->m_xkb_state,
                static_cast<xkb_state_component>(XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED |
                                                 XKB_STATE_MODS_LOCKED)));
        }
        if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            self->m_windowEventBus->dispatch_immediate(std::make_shared<KeyDownEvent>(k, mods, scancode, false));
        } else {
            self->m_windowEventBus->dispatch_immediate(std::make_shared<KeyUpEvent>(k, mods, scancode));
        }
    }

    void WaylandBackend::wl_keyboard_modifiers(void *data, wl_keyboard * /*keyboard*/, uint32_t /*serial*/,
                                              uint32_t mods_depressed, uint32_t mods_latched,
                                              uint32_t mods_locked, uint32_t /*group*/) {
        auto *self = static_cast<WaylandBackend *>(data);
        if (self->m_xkb_state) {
            xkb_state_update_mask(self->m_xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, 0);
        }
    }

    Result<void, WindowBackend::Failure> WaylandBackend::poll_events() {
        if (!m_display)
            return Ok();
        while (wl_display_prepare_read(m_display) != 0) {
            wl_display_dispatch_pending(m_display);
        }
        wl_display_flush(m_display);
        struct pollfd fds[1] = {{wl_display_get_fd(m_display), POLLIN}};
        if (poll(fds, 1, 0) > 0) {
            wl_display_read_events(m_display);
        } else {
            wl_display_cancel_read(m_display);
        }
        wl_display_dispatch_pending(m_display);
        return Ok();
    }

    Result<void, WindowBackend::Failure> WaylandBackend::shutdown() {
        release_input();
        if (m_xdg_toplevel)
            xdg_toplevel_destroy(m_xdg_toplevel);
        m_xdg_toplevel = nullptr;
        if (m_xdg_surface)
            xdg_surface_destroy(m_xdg_surface);
        m_xdg_surface = nullptr;
        if (m_surface)
            wl_surface_destroy(m_surface);
        m_surface = nullptr;
        if (m_xdg_wm_base)
            xdg_wm_base_destroy(m_xdg_wm_base);
        m_xdg_wm_base = nullptr;
        if (m_compositor)
            wl_compositor_destroy(m_compositor);
        m_compositor = nullptr;
        if (m_seat)
            wl_seat_destroy(m_seat);
        m_seat = nullptr;
        if (m_registry)
            wl_registry_destroy(m_registry);
        m_registry = nullptr;
        if (m_display) {
            wl_display_disconnect(m_display);
            m_display = nullptr;
        }
        return Ok();
    }

} // namespace backend
