#pragma once
#include "../../backend/common/backend_types.h"
#include "../../backend/common/render_backend.h"
#include "ribble/core/fail.h"
#include "ribble/render/color.h"

namespace ribble::render {

    enum class RendererFailure {
        InitializationFailure,
        DrawFailure,
    };

    /// High-level, backend-agnostic Renderer class
    /// Wraps RenderBackend and provides convenient rendering interface
    class Renderer {
    public:
        explicit Renderer(backend::RenderBackend &backend);

        /// Begin a new frame (clears buffers)
        core::Result<void, RendererFailure> begin_frame();

        /// End frame (presents to screen)
        core::Result<void, RendererFailure> end_frame();

        /// Set viewport
        void set_viewport(int x, int y, int width, int height);

        /// Set clear color
        void set_clear_color(const ColorRGBA &color);
        void set_clear_color(float r, float g, float b, float a = 1.0f);

        /// Clear buffers
        void clear(bool color = true, bool depth = true, bool stencil = false);

        // ── Render State ────────────────────────────────────────────────────────

        /// Depth testing
        void enable_depth_test();
        void disable_depth_test();
        void set_depth_func(backend::DepthFunc func);
        void set_depth_write(bool enabled);

        /// Blending
        void enable_blend();
        void disable_blend();
        void set_blend_func(backend::BlendFactor src, backend::BlendFactor dst);
        void set_blend_op(backend::BlendOp op);

        /// Face culling
        void enable_cull_face();
        void disable_cull_face();
        void set_cull_mode(backend::CullMode mode);
        void set_winding_order(backend::WindingOrder order);

        /// Program point size
        void enable_program_point_size();
        void disable_program_point_size();

        // ── Drawing ──────────────────────────────────────────────────────────────

        /// Draw indexed geometry
        core::Result<void, RendererFailure>
        draw_indexed(backend::RenderHandle vertexArrayHandle, uint32_t indexCount,
                     backend::IndexType indexType = backend::IndexType::UInt32, uint32_t indexOffset = 0,
                     int32_t baseVertex = 0,
                     backend::PrimitiveTopology topology = backend::PrimitiveTopology::Triangles);

        /// Draw non-indexed geometry
        core::Result<void, RendererFailure>
        draw_arrays(backend::RenderHandle vertexArrayHandle, uint32_t vertexCount, uint32_t vertexOffset = 0,
                    backend::PrimitiveTopology topology = backend::PrimitiveTopology::Triangles);

        /// Draw instanced geometry
        core::Result<void, RendererFailure>
        draw_instanced(backend::RenderHandle vertexArrayHandle, uint32_t indexCount, uint32_t instanceCount,
                       backend::IndexType indexType = backend::IndexType::UInt32, uint32_t indexOffset = 0,
                       int32_t baseVertex = 0,
                       backend::PrimitiveTopology topology = backend::PrimitiveTopology::Triangles);

        /// Get the underlying render backend
        [[nodiscard]] backend::RenderBackend &backend() { return m_backend; }
        [[nodiscard]] const backend::RenderBackend &backend() const { return m_backend; }

    private:
        backend::RenderBackend &m_backend;
        bool m_inFrame{false};
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(
        ribble::render::RendererFailure,
        case ribble::render::RendererFailure::InitializationFailure : return "Renderer Initialization Failure";
        case ribble::render::RendererFailure::DrawFailure : return "Renderer Draw Failure";);
