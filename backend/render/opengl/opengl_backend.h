#pragma once
#include <memory>
#include <unordered_map>
#include "../../common/render_backend.h"
#include "opengl_buffer.h"
#include "opengl_context.h"
#include "opengl_framebuffer.h"
#include "opengl_shader.h"
#include "opengl_state.h"
#include "opengl_texture.h"
#include "ribble/render/color.h"

namespace backend {

    class OpenGLBackend : public RenderBackend {
    public:
        explicit OpenGLBackend(std::unique_ptr<OpenGLContext> context);
        ~OpenGLBackend() override;

        OpenGLBackend(const OpenGLBackend &) = delete;
        OpenGLBackend &operator=(const OpenGLBackend &) = delete;

        ribble::core::Result<void, Failure> initialize(ribble::window::WindowContext &windowContext, int width,
                                                       int height) override;
        ribble::core::Result<void, Failure> shutdown() override;

        ribble::core::Result<void, Failure> begin_frame() override;
        ribble::core::Result<void, Failure> end_frame() override;

        ribble::core::Result<void, Failure> set_viewport(const Viewport &viewport) override;
        ribble::core::Result<void, Failure> set_clear_color(const ribble::render::ColorRGBA &color) override;
        ribble::core::Result<void, Failure> clear() override;

        ribble::core::Result<void, Failure> on_resize(int width, int height) override;

        // Resource creation
        ribble::core::Result<RenderHandle, Failure> create_shader(const ShaderSource &source) override;
        void destroy_shader(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_texture(int width, int height, TextureFormat format,
                                                                   const void *data = nullptr) override;
        void destroy_texture(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_buffer(BufferType type, BufferUsage usage, size_t size,
                                                                  const void *data = nullptr) override;
        void destroy_buffer(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_vertex_array() override;
        void destroy_vertex_array(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_framebuffer() override;
        void destroy_framebuffer(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_pipeline(RenderHandle shaderHandle) override;
        void destroy_pipeline(RenderHandle handle) override;

        // Resource binding
        ribble::core::Result<void, Failure> bind_pipeline(RenderHandle pipelineHandle) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        int value) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        float value) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y, float z) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y, float z, float w) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        const float *matrixData, bool transpose) override;
        ribble::core::Result<void, Failure> bind_texture(RenderHandle textureHandle, int unit = 0) override;
        ribble::core::Result<void, Failure> bind_buffer(RenderHandle bufferHandle, BufferType type) override;
        ribble::core::Result<void, Failure> bind_vertex_array(RenderHandle vertexArrayHandle) override;
        ribble::core::Result<void, Failure> bind_framebuffer(RenderHandle framebufferHandle) override;

        // Draw methods
        ribble::core::Result<void, Failure> draw_indexed(RenderHandle vertexArrayHandle, uint32_t indexCount,
                                                         IndexType indexType, uint32_t indexOffset, int32_t baseVertex,
                                                         PrimitiveTopology topology) override;
        ribble::core::Result<void, Failure> draw_arrays(RenderHandle vertexArrayHandle, uint32_t vertexCount,
                                                        uint32_t vertexOffset, PrimitiveTopology topology) override;
        ribble::core::Result<void, Failure> draw_instanced(RenderHandle vertexArrayHandle, uint32_t indexCount,
                                                           uint32_t instanceCount, IndexType indexType,
                                                           uint32_t indexOffset, int32_t baseVertex,
                                                           PrimitiveTopology topology) override;

        // Render state methods
        ribble::core::Result<void, Failure> set_depth_test(bool enabled) override;
        ribble::core::Result<void, Failure> set_depth_write(bool enabled) override;
        ribble::core::Result<void, Failure> set_depth_func(DepthFunc func) override;
        ribble::core::Result<void, Failure> set_blend(bool enabled) override;
        ribble::core::Result<void, Failure> set_blend_func(BlendFactor src, BlendFactor dst) override;
        ribble::core::Result<void, Failure> set_blend_op(BlendOp op) override;
        ribble::core::Result<void, Failure> set_cull_mode(CullMode mode) override;
        ribble::core::Result<void, Failure> set_winding_order(WindingOrder order) override;
        ribble::core::Result<void, Failure> set_program_point_size(bool enabled) override;

        [[nodiscard]] const char *backend_name() const override { return m_context->backend_name(); }

        [[nodiscard]] OpenGLState &state() { return m_state; }
        [[nodiscard]] const OpenGLState &state() const { return m_state; }
        [[nodiscard]] int framebuffer_width() const { return m_fbWidth; }
        [[nodiscard]] int framebuffer_height() const { return m_fbHeight; }

    private:
        std::unique_ptr<OpenGLContext> m_context;
        OpenGLState m_state;
        ribble::render::ColorRGBA m_clearColor{0.1f, 0.1f, 0.1f, 1.f};
        int m_fbWidth{0};
        int m_fbHeight{0};

        // Resource management
        RenderHandle m_nextShaderHandle{1};
        RenderHandle m_nextTextureHandle{1};
        RenderHandle m_nextBufferHandle{1};
        RenderHandle m_nextVertexArrayHandle{1};
        RenderHandle m_nextFramebufferHandle{1};
        RenderHandle m_nextPipelineHandle{1};
        std::unordered_map<RenderHandle, std::unique_ptr<OpenGLShader>> m_shaders;
        std::unordered_map<RenderHandle, std::unique_ptr<OpenGLTexture>> m_textures;
        std::unordered_map<RenderHandle, std::unique_ptr<OpenGLBuffer>> m_buffers;
        std::unordered_map<RenderHandle, std::unique_ptr<OpenGLVertexArray>> m_vertexArrays;
        std::unordered_map<RenderHandle, std::unique_ptr<OpenGLFramebuffer>> m_framebuffers;
        std::unordered_map<RenderHandle, RenderHandle> m_pipelines; // Pipeline handle -> Shader handle mapping

        static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                                 const GLchar *message, const void *userParam);
    };

} // namespace backend
