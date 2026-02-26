#pragma once
#include <ribble/core/fail.h>

#include <memory>
#include <utility>

#include "ribble/core/event.h"

namespace ribble::backend {

    enum class WindowBackendType {
        SDL3,
        GLFW, // TODO
        // ... others here
    };

    class WindowBackend {
    public:
        enum class Failure { InitializationFailure, ShutdownFailure };
        WindowBackend(std::shared_ptr<core::EventBus> windowEventBus) : m_windowEventBus{std::move(windowEventBus)} {}
        virtual ~WindowBackend() = default;

        virtual core::Result<void, Failure> initialize(int width, int height, const char *title) {
            RIBBLE_LOG_INFO("Initializing window.");
            return core::Ok();
        }
        virtual core::Result<void, Failure> poll_events() = 0;
        virtual core::Result<void, Failure> shutdown() {
            RIBBLE_LOG_INFO("Closing window.");
            return core::Ok();
        }
        [[nodiscard]] virtual void *native_handle() const = 0;

    protected:
        std::shared_ptr<core::EventBus> m_windowEventBus;
    };

} // namespace ribble::backend

RIBBLE_ENUM_TO_STRING(
        ribble::backend::WindowBackend::Failure,
        case ribble::backend::WindowBackend::Failure::InitializationFailure : return "Initialization Failure";);
