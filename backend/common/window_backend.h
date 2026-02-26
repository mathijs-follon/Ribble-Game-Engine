#pragma once
#include <ribble/core/fail.h>

#include <memory>
#include <utility>

#include "backend_types.h"
#include "ribble/core/event.h"

namespace backend {

    enum class WindowBackendType {
        SDL3,
        X11,
        GLFW,
        Wayland,
        Win32,
    };

    class WindowBackend {
    public:
        enum class Failure { InitializationFailure, ShutdownFailure };
        WindowBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus) :
            m_windowEventBus{std::move(windowEventBus)} {}
        virtual ~WindowBackend() = default;

        /// Set graphics API hint before initialize (affects window creation flags)
        virtual void set_graphics_api(GraphicsAPI api) { m_graphicsAPI = api; }

        virtual ribble::core::Result<void, Failure> initialize(int width, int height, const char *title) {
            RIBBLE_LOG_INFO("Initializing window.");
            return ribble::core::Ok();
        }
        virtual ribble::core::Result<void, Failure> poll_events() = 0;
        virtual ribble::core::Result<void, Failure> shutdown() {
            RIBBLE_LOG_INFO("Closing window.");
            return ribble::core::Ok();
        }
        [[nodiscard]] virtual void *native_handle() const = 0;

        /// Optional: display/connection handle (e.g. wl_display for Wayland). Returns nullptr if not applicable.
        [[nodiscard]] virtual void *native_display_handle() const { return nullptr; }

    protected:
        std::shared_ptr<ribble::core::EventBus> m_windowEventBus;
        GraphicsAPI m_graphicsAPI{GraphicsAPI::OpenGL};
    };

} // namespace backend

using WindowBackendFailure = backend::WindowBackend::Failure;

RIBBLE_ENUM_TO_STRING(WindowBackendFailure,
                      case WindowBackendFailure::InitializationFailure : return "Initialization Failure";);
