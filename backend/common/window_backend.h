#pragma once
#include <ribble/core/fail.h>

#include <memory>
#include <utility>

#include "ribble/core/event.h"

namespace backend {

    enum class WindowBackendType {
        SDL3,
        X11,
        GLFW,
    };

    class WindowBackend {
    public:
        enum class Failure { InitializationFailure, ShutdownFailure };
        WindowBackend(std::shared_ptr<ribble::core::EventBus> windowEventBus) :
            m_windowEventBus{std::move(windowEventBus)} {}
        virtual ~WindowBackend() = default;

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

    protected:
        std::shared_ptr<ribble::core::EventBus> m_windowEventBus;
    };

} // namespace backend

using WindowBackendFailure = backend::WindowBackend::Failure;

RIBBLE_ENUM_TO_STRING(WindowBackendFailure,
                      case WindowBackendFailure::InitializationFailure : return "Initialization Failure";);
