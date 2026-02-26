#pragma once

#include <utility>
#include <wayland-client.h>
#include "../../common/window_backend.h"

struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;

struct xkb_state;
struct xkb_context;
struct xkb_keymap;

namespace backend {

    class WaylandBackend : public WindowBackend {
    public:
        explicit WaylandBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus);
        ~WaylandBackend() override;

        ribble::core::Result<void, Failure> initialize(int width, int height, const char *title) override;
        ribble::core::Result<void, Failure> poll_events() override;
        ribble::core::Result<void, Failure> shutdown() override;

        [[nodiscard]] void *native_handle() const override { return m_surface; }
        [[nodiscard]] void *native_display_handle() const override { return m_display; }

    private:
        void setup_input();
        void release_input();

        static void global_registry_handler(void *data, wl_registry *registry, uint32_t id,
                                           const char *interface, uint32_t version);
        static void global_registry_remover(void *data, wl_registry *registry, uint32_t id);

        static void xdg_wm_base_ping(void *data, xdg_wm_base *wm_base, uint32_t serial);
        static void xdg_surface_configure(void *data, xdg_surface *surface, uint32_t serial);
        static void xdg_toplevel_configure(void *data, xdg_toplevel *toplevel, int32_t width, int32_t height,
                                          struct wl_array *states);
        static void xdg_toplevel_close(void *data, xdg_toplevel *toplevel);

        static void wl_pointer_enter(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface,
                                    wl_fixed_t x, wl_fixed_t y);
        static void wl_pointer_leave(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface);
        static void wl_pointer_motion(void *data, wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
        static void wl_pointer_button(void *data, wl_pointer *pointer, uint32_t serial, uint32_t time,
                                     uint32_t button, uint32_t state);
        static void wl_pointer_axis(void *data, wl_pointer *pointer, uint32_t time, uint32_t axis,
                                   wl_fixed_t value);

        static void wl_keyboard_keymap(void *data, wl_keyboard *keyboard, uint32_t format, int fd,
                                      uint32_t size);
        static void wl_keyboard_enter(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface,
                                     wl_array *keys);
        static void wl_keyboard_leave(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
        static void wl_keyboard_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time,
                                   uint32_t key, uint32_t state);
        static void wl_keyboard_modifiers(void *data, wl_keyboard *keyboard, uint32_t serial,
                                         uint32_t mods_depressed, uint32_t mods_latched,
                                         uint32_t mods_locked, uint32_t group);

        // Wayland Core
        wl_display *m_display = nullptr;
        wl_registry *m_registry = nullptr;
        wl_compositor *m_compositor = nullptr;
        wl_surface *m_surface = nullptr;
        wl_seat *m_seat = nullptr;
        wl_pointer *m_pointer = nullptr;
        wl_keyboard *m_keyboard = nullptr;

        // XDG Shell
        xdg_wm_base *m_xdg_wm_base = nullptr;
        xdg_surface *m_xdg_surface = nullptr;
        xdg_toplevel *m_xdg_toplevel = nullptr;

        // XKB for key translation
        xkb_context *m_xkb_context = nullptr;
        xkb_state *m_xkb_state = nullptr;
        xkb_keymap *m_xkb_keymap = nullptr;

        // State
        int m_width = 0;
        int m_height = 0;
        const char *m_title = nullptr;
        bool m_configured = false;
        bool m_closed = false;
        double m_pointerX = 0;
        double m_pointerY = 0;
        bool m_firstMotion = true;
        double m_lastMouseX = 0;
        double m_lastMouseY = 0;
    };

} // namespace backend
