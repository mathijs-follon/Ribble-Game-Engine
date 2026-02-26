#pragma once
#include <memory>
#include <ribble/core/fail.h>
#include <string>

#include "backend_types.h"
#include "ribble/render/color.h"
#include "ribble/window/window.h"
#include "shader_source.h"

namespace backend {

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

        virtual ribble::core::Result<void, Failure> initialize(ribble::window::WindowContext &windowContext, int width,
                                                               int height) {
            RIBBLE_LOG_INFO("Initializing render backend.");
            return ribble::core::Ok();
        }

        virtual ribble::core::Result<void, Failure> shutdown() {
            RIBBLE_LOG_INFO("Shutting down render backend.");
            return ribble::core::Ok();
        }

        /// Called at the start of each frame before any draw calls.
        virtual ribble::core::Result<void, Failure> begin_frame() = 0;

        /// Called at the end of each frame, swaps buffers / presents swapchain.
        virtual ribble::core::Result<void, Failure> end_frame() = 0;

        virtual ribble::core::Result<void, Failure> set_viewport(const Viewport &viewport) = 0;
        virtual ribble::core::Result<void, Failure> set_clear_color(const ribble::render::ColorRGBA &color) = 0;
        virtual ribble::core::Result<void, Failure> clear() = 0;

        /// Must be called whenever the window framebuffer size changes.
        virtual ribble::core::Result<void, Failure> on_resize(int width, int height) = 0;

        [[nodiscard]] virtual const char *backend_name() const = 0;
        [[nodiscard]] virtual bool is_initialized() const { return m_initialized; }

        // ── Resource Creation ───────────────────────────────────────────────────

        /// Create a shader from shader source (language-agnostic)
        /// @param source Shader source (GLSL, HLSL, MSL, or SPIR-V)
        /// @return Handle to the created shader, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_shader(const ShaderSource &source) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::ShaderCompilationFailure, "create_shader not implemented"));
        }

        /// Destroy a shader resource
        virtual void destroy_shader(RenderHandle handle) {}

        /// Create a texture
        /// @param width Texture width
        /// @param height Texture height
        /// @param format Texture format
        /// @param data Optional initial data (nullptr for uninitialized)
        /// @return Handle to the created texture, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_texture(int width, int height, TextureFormat format,
                                                                           const void *data = nullptr) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::TextureCreationFailure, "create_texture not implemented"));
        }

        /// Destroy a texture resource
        virtual void destroy_texture(RenderHandle handle) {}

        /// Create a buffer
        /// @param type Buffer type (Vertex, Index, Uniform, etc.)
        /// @param usage Buffer usage hint
        /// @param size Size in bytes
        /// @param data Optional initial data (nullptr for uninitialized)
        /// @return Handle to the created buffer, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_buffer(BufferType type, BufferUsage usage,
                                                                          size_t size, const void *data = nullptr) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::BufferCreationFailure, "create_buffer not implemented"));
        }

        /// Destroy a buffer resource
        virtual void destroy_buffer(RenderHandle handle) {}

        /// Create a vertex array object (VAO)
        /// @return Handle to the created VAO, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_vertex_array() {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::BufferCreationFailure, "create_vertex_array not implemented"));
        }

        /// Destroy a vertex array resource
        virtual void destroy_vertex_array(RenderHandle handle) {}

        /// Create a framebuffer
        /// @return Handle to the created framebuffer, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_framebuffer() {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::FramebufferCreationFailure, "create_framebuffer not implemented"));
        }

        /// Destroy a framebuffer resource
        virtual void destroy_framebuffer(RenderHandle handle) {}

        /// Create a pipeline state object (for Vulkan compatibility)
        /// OpenGL backends can create a lightweight "pipeline" that binds shader + state
        /// @param shaderHandle Handle to the shader program
        /// @return Handle to the created pipeline, or error
        virtual ribble::core::Result<RenderHandle, Failure> create_pipeline(RenderHandle shaderHandle) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "create_pipeline not implemented"));
        }

        /// Destroy a pipeline resource
        virtual void destroy_pipeline(RenderHandle handle) {}

        // ── Draw Methods ────────────────────────────────────────────────────────

        /// Draw indexed geometry (immediate mode)
        /// @param vertexArrayHandle Handle to vertex array
        /// @param indexCount Number of indices to draw
        /// @param indexType Type of index data
        /// @param indexOffset Offset into index buffer
        /// @param baseVertex Base vertex for indexed drawing
        /// @param topology Primitive topology
        virtual ribble::core::Result<void, Failure>
        draw_indexed(RenderHandle vertexArrayHandle, uint32_t indexCount, IndexType indexType, uint32_t indexOffset = 0,
                     int32_t baseVertex = 0, PrimitiveTopology topology = PrimitiveTopology::Triangles) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "draw_indexed not implemented"));
        }

        /// Draw non-indexed geometry (immediate mode)
        /// @param vertexArrayHandle Handle to vertex array
        /// @param vertexCount Number of vertices to draw
        /// @param vertexOffset Offset into vertex buffer
        /// @param topology Primitive topology
        virtual ribble::core::Result<void, Failure>
        draw_arrays(RenderHandle vertexArrayHandle, uint32_t vertexCount, uint32_t vertexOffset = 0,
                    PrimitiveTopology topology = PrimitiveTopology::Triangles) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "draw_arrays not implemented"));
        }

        /// Draw instanced geometry (immediate mode)
        /// @param vertexArrayHandle Handle to vertex array
        /// @param indexCount Number of indices per instance (0 for non-indexed)
        /// @param instanceCount Number of instances
        /// @param indexType Type of index data (if indexed)
        /// @param indexOffset Offset into index buffer
        /// @param baseVertex Base vertex for indexed drawing
        /// @param topology Primitive topology
        virtual ribble::core::Result<void, Failure>
        draw_instanced(RenderHandle vertexArrayHandle, uint32_t indexCount, uint32_t instanceCount,
                       IndexType indexType = IndexType::UInt32, uint32_t indexOffset = 0, int32_t baseVertex = 0,
                       PrimitiveTopology topology = PrimitiveTopology::Triangles) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "draw_instanced not implemented"));
        }

        // ── Render State ────────────────────────────────────────────────────────

        /// Set depth test state
        virtual ribble::core::Result<void, Failure> set_depth_test(bool enabled) { return ribble::core::Ok(); }

        /// Set depth write (mask) state
        virtual ribble::core::Result<void, Failure> set_depth_write(bool enabled) { return ribble::core::Ok(); }

        /// Set depth comparison function
        virtual ribble::core::Result<void, Failure> set_depth_func(DepthFunc func) { return ribble::core::Ok(); }

        /// Set blend state
        virtual ribble::core::Result<void, Failure> set_blend(bool enabled) { return ribble::core::Ok(); }

        /// Set blend function
        virtual ribble::core::Result<void, Failure> set_blend_func(BlendFactor src, BlendFactor dst) {
            return ribble::core::Ok();
        }

        /// Set blend operation
        virtual ribble::core::Result<void, Failure> set_blend_op(BlendOp op) { return ribble::core::Ok(); }

        /// Set cull mode
        virtual ribble::core::Result<void, Failure> set_cull_mode(CullMode mode) { return ribble::core::Ok(); }

        /// Set winding order
        virtual ribble::core::Result<void, Failure> set_winding_order(WindingOrder order) { return ribble::core::Ok(); }

        /// Set program point size (for point sprites)
        virtual ribble::core::Result<void, Failure> set_program_point_size(bool enabled) { return ribble::core::Ok(); }

        // ── Resource Binding ────────────────────────────────────────────────────

        /// Set uniform value (int)
        /// @param shaderHandle Handle to shader program
        /// @param name Uniform name
        /// @param value Value to set
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                int value) {
            return ribble::core::Ok();
        }

        /// Set uniform value (float)
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                float value) {
            return ribble::core::Ok();
        }

        /// Set uniform value (vec2)
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                float x, float y) {
            return ribble::core::Ok();
        }

        /// Set uniform value (vec3)
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                float x, float y, float z) {
            return ribble::core::Ok();
        }

        /// Set uniform value (vec4)
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                float x, float y, float z, float w) {
            return ribble::core::Ok();
        }

        /// Set uniform value (mat4)
        virtual ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                                const float *matrixData, bool transpose = false) {
            return ribble::core::Ok();
        }

        /// Bind texture to a texture unit
        /// @param textureHandle Handle to texture
        /// @param unit Texture unit index (0-31)
        virtual ribble::core::Result<void, Failure> bind_texture(RenderHandle textureHandle, int unit = 0) {
            return ribble::core::Ok();
        }

        /// Bind buffer
        /// @param bufferHandle Handle to buffer
        /// @param type Buffer type (for OpenGL target selection)
        virtual ribble::core::Result<void, Failure> bind_buffer(RenderHandle bufferHandle, BufferType type) {
            return ribble::core::Ok();
        }

        /// Bind vertex array
        /// @param vertexArrayHandle Handle to vertex array
        virtual ribble::core::Result<void, Failure> bind_vertex_array(RenderHandle vertexArrayHandle) {
            return ribble::core::Ok();
        }

        /// Bind framebuffer
        /// @param framebufferHandle Handle to framebuffer (InvalidHandle for default)
        virtual ribble::core::Result<void, Failure> bind_framebuffer(RenderHandle framebufferHandle) {
            return ribble::core::Ok();
        }

        /// Bind pipeline (shader + state)
        /// @param pipelineHandle Handle to pipeline
        virtual ribble::core::Result<void, Failure> bind_pipeline(RenderHandle pipelineHandle) {
            return ribble::core::Ok();
        }

    protected:
        bool m_initialized{false};
    };

} // namespace backend

using RenderBackendFailure = backend::RenderBackend::Failure;

RIBBLE_ENUM_TO_STRING(RenderBackendFailure,
                      case RenderBackendFailure::InitializationFailure : return "Initialization Failure";
                      case RenderBackendFailure::ShutdownFailure : return "Shutdown Failure";
                      case RenderBackendFailure::ContextCreationFailure : return "Context Creation Failure";
                      case RenderBackendFailure::ShaderCompilationFailure : return "Shader Compilation Failure";
                      case RenderBackendFailure::BufferCreationFailure : return "Buffer Creation Failure";
                      case RenderBackendFailure::TextureCreationFailure : return "Texture Creation Failure";
                      case RenderBackendFailure::FramebufferCreationFailure : return "Framebuffer Creation Failure";
                      case RenderBackendFailure::DrawFailure : return "Draw Failure";);
