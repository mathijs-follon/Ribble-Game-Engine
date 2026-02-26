#pragma once
#include <memory>
#include <ribble/core/fail.h>

#include "ribble/render/color.h"

namespace ribble::backend {

    enum class RenderBackendType {
        OpenGL,
        Vulkan,
        DirectX12,
        Metal,
    };

    struct Viewport {
        int x{0};
        int y{0};
        int width{0};
        int height{0};
    };

    class RenderBackend {
    public:
        enum class Failure {
            InitializationFailure,
            ShutdownFailure,
            ContextCreationFailure,
            ShaderCompilationFailure,
            BufferCreationFailure,
            TextureCreationFailure,
            FramebufferCreationFailure,
            DrawFailure,
        };

        explicit RenderBackend() = default;
        virtual ~RenderBackend() = default;

        RenderBackend(const RenderBackend &) = delete;
        RenderBackend &operator=(const RenderBackend &) = delete;
        RenderBackend(RenderBackend &&) = delete;
        RenderBackend &operator=(RenderBackend &&) = delete;

        /// Initialize the render backend against a native window handle.
        /// @param nativeWindowHandle   Platform window handle (HWND, Display*, etc.)
        /// @param width                Initial framebuffer width in pixels
        /// @param height               Initial framebuffer height in pixels
        virtual core::Result<void, Failure> initialize(void *nativeWindowHandle, int width, int height) {
            RIBBLE_LOG_INFO("Initializing render backend.");
            return core::Ok();
        }

        virtual core::Result<void, Failure> shutdown() {
            RIBBLE_LOG_INFO("Shutting down render backend.");
            return core::Ok();
        }

        /// Called at the start of each frame before any draw calls.
        virtual core::Result<void, Failure> begin_frame() = 0;

        /// Called at the end of each frame, swaps buffers / presents swapchain.
        virtual core::Result<void, Failure> end_frame() = 0;

        virtual core::Result<void, Failure> set_viewport(const Viewport &viewport) = 0;
        virtual core::Result<void, Failure> set_clear_color(const render::ColorRGBA &color) = 0;
        virtual core::Result<void, Failure> clear() = 0;

        /// Must be called whenever the window framebuffer size changes.
        virtual core::Result<void, Failure> on_resize(int width, int height) = 0;

        [[nodiscard]] virtual const char *backend_name() const = 0;
        [[nodiscard]] virtual bool is_initialized() const { return m_initialized; }

    protected:
        bool m_initialized{false};
    };

} // namespace ribble::backend

RIBBLE_ENUM_TO_STRING(
        ribble::backend::RenderBackend::Failure,
        case ribble::backend::RenderBackend::Failure::InitializationFailure : return "Initialization Failure";
        case ribble::backend::RenderBackend::Failure::ShutdownFailure : return "Shutdown Failure";
        case ribble::backend::RenderBackend::Failure::ContextCreationFailure : return "Context Creation Failure";
        case ribble::backend::RenderBackend::Failure::ShaderCompilationFailure : return "Shader Compilation Failure";
        case ribble::backend::RenderBackend::Failure::BufferCreationFailure : return "Buffer Creation Failure";
        case ribble::backend::RenderBackend::Failure::TextureCreationFailure : return "Texture Creation Failure";
        case ribble::backend::RenderBackend::Failure::
                FramebufferCreationFailure : return "Framebuffer Creation Failure";
        case ribble::backend::RenderBackend::Failure::DrawFailure : return "Draw Failure";);
