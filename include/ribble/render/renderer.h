#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "ribble/core/fail.h"
#include "ribble/render/color.h"
#include "ribble/scene/scene.h"
#include "ribble/window/window.h"

namespace backend {
    class RenderBackend;
    enum class WindowBackendType;
    enum class RenderBackendType;
} // namespace backend

namespace ribble::render {

    enum class RendererFailure {
        InitializationFailure,
        ShutdownFailure,
        DrawFailure,
    };

    /// Camera view used for culling and MVP calculation.
    struct CameraView {
        glm::mat4 view{1};
        glm::mat4 projection{1};
        int viewportX{0};
        int viewportY{0};
        int viewportWidth{1024};
        int viewportHeight{768};

        [[nodiscard]] glm::mat4 view_projection() const { return projection * view; }
    };

    /// High-level, scene-driven Renderer. Owns the RenderBackend and decides how to
    /// efficiently render a scene. You provide a scene tree and camera; the renderer
    /// collects visible renderables, batches them, and draws.
    class Renderer {
    public:
        Renderer(backend::WindowBackendType windowType, backend::RenderBackendType renderType);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        /// Initialize with window context (call after window is created).
        core::Result<void, RendererFailure> initialize(ribble::window::WindowContext &windowContext, int width,
                                                       int height);

        void shutdown();

        [[nodiscard]] bool is_initialized() const;

        /// Draw the entire scene. Walks the scene tree, collects nodes with renderable
        /// components that are within the camera viewport, batches for efficiency, and draws.
        /// Nodes outside the viewport are culled. Pass nullptr to only clear and present.
        core::Result<void, RendererFailure> draw_scene(scene::Scene *scene, const CameraView &camera);

        /// Set clear color used at frame start.
        void set_clear_color(const ColorRGBA &color);
        void set_clear_color(float r, float g, float b, float a = 1.0f);

        /// Access the underlying backend for low-level operations (resource creation, etc.).
        [[nodiscard]] backend::RenderBackend &backend();
        [[nodiscard]] const backend::RenderBackend &backend() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(
        ribble::render::RendererFailure,
        case ribble::render::RendererFailure::InitializationFailure : return "Renderer Initialization Failure";
        case ribble::render::RendererFailure::ShutdownFailure : return "Renderer Shutdown Failure";
        case ribble::render::RendererFailure::DrawFailure : return "Renderer Draw Failure";);
