#include "ribble/render/renderer.h"
#include "ribble/core/logger.h"

namespace ribble::render {

    Renderer::Renderer(backend::RenderBackend &backend) : m_backend(backend) {}

    core::Result<void, RendererFailure> Renderer::begin_frame() {
        if (m_inFrame) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "Frame already started"));
        }

        auto result = m_backend.begin_frame();
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "Failed to begin frame"));
        }

        m_inFrame = true;
        return core::Ok();
    }

    core::Result<void, RendererFailure> Renderer::end_frame() {
        if (!m_inFrame) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "No frame to end"));
        }

        auto result = m_backend.end_frame();
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "Failed to end frame"));
        }

        m_inFrame = false;
        return core::Ok();
    }

    void Renderer::set_viewport(int x, int y, int width, int height) {
        backend::Viewport vp{x, y, width, height};
        m_backend.set_viewport(vp);
    }

    void Renderer::set_clear_color(const ColorRGBA &color) { m_backend.set_clear_color(color); }

    void Renderer::set_clear_color(float r, float g, float b, float a) {
        m_backend.set_clear_color(ColorRGBA{r, g, b, a});
    }

    void Renderer::clear(bool color, bool depth, bool stencil) { m_backend.clear(); }

    // Render state methods
    void Renderer::enable_depth_test() { m_backend.set_depth_test(true); }

    void Renderer::disable_depth_test() { m_backend.set_depth_test(false); }

    void Renderer::set_depth_func(backend::DepthFunc func) { m_backend.set_depth_func(func); }

    void Renderer::set_depth_write(bool enabled) { m_backend.set_depth_write(enabled); }

    void Renderer::enable_blend() { m_backend.set_blend(true); }

    void Renderer::disable_blend() { m_backend.set_blend(false); }

    void Renderer::set_blend_func(backend::BlendFactor src, backend::BlendFactor dst) {
        m_backend.set_blend_func(src, dst);
    }

    void Renderer::set_blend_op(backend::BlendOp op) { m_backend.set_blend_op(op); }

    void Renderer::enable_cull_face() { m_backend.set_cull_mode(backend::CullMode::Back); }

    void Renderer::disable_cull_face() { m_backend.set_cull_mode(backend::CullMode::None); }

    void Renderer::set_cull_mode(backend::CullMode mode) { m_backend.set_cull_mode(mode); }

    void Renderer::set_winding_order(backend::WindingOrder order) { m_backend.set_winding_order(order); }

    void Renderer::enable_program_point_size() {
        // This would need backend support
        // TODO: Add set_program_point_size to RenderBackend interface if needed
    }

    void Renderer::disable_program_point_size() {
        // This would need backend support
        // TODO: Add set_program_point_size to RenderBackend interface if needed
    }

    core::Result<void, RendererFailure> Renderer::draw_indexed(backend::RenderHandle vertexArrayHandle,
                                                               uint32_t indexCount, backend::IndexType indexType,
                                                               uint32_t indexOffset, int32_t baseVertex,
                                                               backend::PrimitiveTopology topology) {

        auto result =
                m_backend.draw_indexed(vertexArrayHandle, indexCount, indexType, indexOffset, baseVertex, topology);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "Draw indexed failed"));
        }
        return core::Ok();
    }

    core::Result<void, RendererFailure> Renderer::draw_arrays(backend::RenderHandle vertexArrayHandle,
                                                              uint32_t vertexCount, uint32_t vertexOffset,
                                                              backend::PrimitiveTopology topology) {

        auto result = m_backend.draw_arrays(vertexArrayHandle, vertexCount, vertexOffset, topology);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "Draw arrays failed"));
        }
        return core::Ok();
    }

    core::Result<void, RendererFailure> Renderer::draw_instanced(backend::RenderHandle vertexArrayHandle,
                                                                 uint32_t indexCount, uint32_t instanceCount,
                                                                 backend::IndexType indexType, uint32_t indexOffset,
                                                                 int32_t baseVertex,
                                                                 backend::PrimitiveTopology topology) {

        auto result = m_backend.draw_instanced(vertexArrayHandle, indexCount, instanceCount, indexType, indexOffset,
                                               baseVertex, topology);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "Draw instanced failed"));
        }
        return core::Ok();
    }

} // namespace ribble::render
