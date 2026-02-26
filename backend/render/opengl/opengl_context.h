#pragma once
#include <ribble/core/fail.h>
#include "../../common/render_backend.h"
#include "ribble/window/window.h"


namespace backend {
    class OpenGLContext {
    public:
        virtual ~OpenGLContext() = default;

        virtual ribble::core::Result<void, RenderBackend::Failure>
        create(ribble::window::WindowContext &windowContext) = 0;
        virtual void destroy() = 0;
        virtual void swap_buffers() = 0;
        virtual void set_swap_interval(int interval) = 0;

        [[nodiscard]] virtual const char *backend_name() const = 0;
        [[nodiscard]] virtual bool is_valid() const = 0;
    };
} // namespace backend
