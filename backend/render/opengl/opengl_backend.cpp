#include "opengl_backend.h"
#include <ribble/core/logger.h>

#include "../../common/backend_types.h"
#include "../../common/shader_source.h"
#include "../../common/window_events.h"
#include "opengl_conversions.h"

namespace backend {

    OpenGLBackend::OpenGLBackend(std::unique_ptr<OpenGLContext> context) : m_context(std::move(context)) {}

    OpenGLBackend::~OpenGLBackend() {
        if (m_initialized)
            OpenGLBackend::shutdown();
    }

    void GLAPIENTRY OpenGLBackend::gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                                     GLsizei /*length*/, const GLchar *message,
                                                     const void * /*userParam*/) {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        const char *srcStr = [source]() -> const char * {
            switch (source) {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "Window System";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "Shader Compiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "Third Party";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "Application";
                default:
                    return "Other";
            }
        }();

        const char *typeStr = [type]() -> const char * {
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:
                    return "Error";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "Deprecated";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "Undefined Behavior";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "Portability";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "Performance";
                default:
                    return "Other";
            }
        }();

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                RIBBLE_LOG_ERROR("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                RIBBLE_LOG_WARNING("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                RIBBLE_LOG_INFO("[GL][{}][{}][{}] {}", srcStr, typeStr, id, message);
                break;
            default:
                break;
        }
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::initialize(ribble::window::WindowContext &windowContext, int width, int height) {
        RenderBackend::initialize(windowContext, width, height);

        m_fbWidth = width;
        m_fbHeight = height;

        if (auto r = m_context->create(windowContext); !r)
            return ribble::core::Fail(r.error());

        // Get OpenGL version
        int major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        RIBBLE_LOG_INFO("OpenGL {}.{} — Renderer: {} — Vendor: {}", major, minor,
                        reinterpret_cast<const char *>(glGetString(GL_RENDERER)),
                        reinterpret_cast<const char *>(glGetString(GL_VENDOR)));

#if defined(RIBBLE_DEBUG)
        if (GLAD_GL_KHR_debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(gl_debug_callback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            RIBBLE_LOG_INFO("OpenGL debug output enabled.");
        }
#endif

        m_state.reset();
        m_state.set_viewport({0, 0, width, height});
        m_state.set_clear_color(m_clearColor);
        m_state.set_depth_test(true);
        m_state.set_depth_func(DepthFunc::Less);
        m_state.set_cull_face(CullMode::Back);
        m_state.set_winding_order(WindingOrder::CounterClockwise);

        windowContext.event_bus()->subscribe<WindowResizeEvent>(
                [this](const std::shared_ptr<ribble::core::Event> &baseEvt) {
                    const auto &evt = static_cast<const WindowResizeEvent &>(*baseEvt);
                    on_resize(evt.width(), evt.height());
                });

        m_initialized = true;
        RIBBLE_LOG_DEBUG("OpenGL backend initialized ({}x{}).", width, height);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::shutdown() {
        if (!m_initialized)
            return ribble::core::Ok();
        m_context->destroy();
        m_initialized = false;
        RIBBLE_LOG_INFO("OpenGL backend shut down.");
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::begin_frame() {
        m_state.bind_framebuffer(GL_FRAMEBUFFER, 0);
        m_state.set_viewport({0, 0, m_fbWidth, m_fbHeight});
        m_state.clear(true, true, true);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::end_frame() {
        m_context->swap_buffers();
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_viewport(const Viewport &viewport) {
        m_state.set_viewport(viewport);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::set_clear_color(const ribble::render::ColorRGBA &color) {
        m_clearColor = color;
        m_state.set_clear_color(color);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::clear() {
        m_state.clear(true, true, true);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::on_resize(int width, int height) {
        m_fbWidth = width;
        m_fbHeight = height;
        m_state.set_viewport({0, 0, width, height});
        return ribble::core::Ok();
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    OpenGLBackend::create_shader(const ShaderSource &source) {
        if (source.language != ShaderLanguage::GLSL) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "OpenGL backend only supports GLSL shaders"));
        }

        auto shader = std::make_unique<OpenGLShader>();
        auto compileResult = shader->compile(source);
        if (!compileResult) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Shader compilation failed"));
        }

        // For multi-stage shaders, we need to link them
        // For now, we'll assume single-stage shaders are compiled and linked immediately
        // In a full implementation, we'd need to track multiple stages and link them together
        auto linkResult = shader->link();
        if (!linkResult) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Shader linking failed"));
        }

        RenderHandle handle = m_nextShaderHandle++;
        m_shaders[handle] = std::move(shader);
        return ribble::core::Ok(handle);
    }

    void OpenGLBackend::destroy_shader(RenderHandle handle) {
        auto it = m_shaders.find(handle);
        if (it != m_shaders.end()) {
            it->second->destroy();
            m_shaders.erase(it);
        }
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::bind_pipeline(RenderHandle pipelineHandle) {
        if (pipelineHandle == InvalidHandle) {
            glUseProgram(0);
            m_state.use_program(0);
            return ribble::core::Ok();
        }

        // For OpenGL, pipeline handle maps to shader handle
        auto pipelineIt = m_pipelines.find(pipelineHandle);
        if (pipelineIt == m_pipelines.end()) {
            // If not found in pipelines, try direct shader handle
            auto shaderIt = m_shaders.find(pipelineHandle);
            if (shaderIt == m_shaders.end()) {
                return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid pipeline handle"));
            }
            shaderIt->second->bind();
            m_state.use_program(shaderIt->second->program_id());
            return ribble::core::Ok();
        }

        // Get the shader handle from pipeline
        RenderHandle shaderHandle = pipelineIt->second;
        auto shaderIt = m_shaders.find(shaderHandle);
        if (shaderIt == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle in pipeline"));
        }

        shaderIt->second->bind();
        m_state.use_program(shaderIt->second->program_id());
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_uniform(RenderHandle shaderHandle,
                                                                                  const std::string &name, int value) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        it->second->set_int(name, value);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::set_uniform(RenderHandle shaderHandle, const std::string &name, float value) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        it->second->set_float(name, value);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::set_uniform(RenderHandle shaderHandle, const std::string &name, float x, float y) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        it->second->set_vec2(name, x, y);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::set_uniform(RenderHandle shaderHandle, const std::string &name, float x, float y, float z) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        it->second->set_vec3(name, x, y, z);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::set_uniform(RenderHandle shaderHandle, const std::string &name, float x, float y, float z, float w) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        it->second->set_vec4(name, x, y, z, w);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_uniform(RenderHandle shaderHandle,
                                                                                  const std::string &name,
                                                                                  const float *matrixData,
                                                                                  bool transpose) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid shader handle"));
        }
        // Assume mat4 for now (16 floats)
        it->second->set_mat4(name, matrixData, transpose);
        return ribble::core::Ok();
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    OpenGLBackend::create_texture(int width, int height, TextureFormat format, const void *data) {
        TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.data = data;
        desc.generateMipmaps = (data != nullptr);
        desc.minFilter = desc.generateMipmaps ? TextureFilter::LinearMipmapLinear : TextureFilter::Linear;
        desc.magFilter = TextureFilter::Linear;
        desc.wrapS = TextureWrap::Repeat;
        desc.wrapT = TextureWrap::Repeat;

        auto texture = std::make_unique<OpenGLTexture>();
        auto result = texture->create(desc);
        if (!result) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::TextureCreationFailure, "Failed to create texture"));
        }

        RenderHandle handle = m_nextTextureHandle++;
        m_textures[handle] = std::move(texture);
        return ribble::core::Ok(handle);
    }

    void OpenGLBackend::destroy_texture(RenderHandle handle) {
        auto it = m_textures.find(handle);
        if (it != m_textures.end()) {
            it->second->destroy();
            m_textures.erase(it);
        }
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::bind_texture(RenderHandle textureHandle,
                                                                                   int unit) {
        if (textureHandle == InvalidHandle) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, 0);
            return ribble::core::Ok();
        }

        auto it = m_textures.find(textureHandle);
        if (it == m_textures.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid texture handle"));
        }

        it->second->bind(unit);
        return ribble::core::Ok();
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    OpenGLBackend::create_buffer(BufferType type, BufferUsage usage, size_t size, const void *data) {
        auto buffer = std::make_unique<OpenGLBuffer>();
        auto result = buffer->create(type, usage, data, size);
        if (!result) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::BufferCreationFailure, "Failed to create buffer"));
        }

        RenderHandle handle = m_nextBufferHandle++;
        m_buffers[handle] = std::move(buffer);
        return ribble::core::Ok(handle);
    }

    void OpenGLBackend::destroy_buffer(RenderHandle handle) {
        auto it = m_buffers.find(handle);
        if (it != m_buffers.end()) {
            it->second->destroy();
            m_buffers.erase(it);
        }
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure> OpenGLBackend::create_vertex_array() {
        auto vertexArray = std::make_unique<OpenGLVertexArray>();
        vertexArray->create();

        RenderHandle handle = m_nextVertexArrayHandle++;
        m_vertexArrays[handle] = std::move(vertexArray);
        return ribble::core::Ok(handle);
    }

    void OpenGLBackend::destroy_vertex_array(RenderHandle handle) {
        auto it = m_vertexArrays.find(handle);
        if (it != m_vertexArrays.end()) {
            it->second->destroy();
            m_vertexArrays.erase(it);
        }
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure> OpenGLBackend::create_framebuffer() {
        auto framebuffer = std::make_unique<OpenGLFramebuffer>();
        // Create a default framebuffer (empty, can be configured later)
        // For now, we'll just create an empty framebuffer handle
        // In a full implementation, we'd need a way to configure the framebuffer

        RenderHandle handle = m_nextFramebufferHandle++;
        m_framebuffers[handle] = std::move(framebuffer);
        return ribble::core::Ok(handle);
    }

    void OpenGLBackend::destroy_framebuffer(RenderHandle handle) {
        auto it = m_framebuffers.find(handle);
        if (it != m_framebuffers.end()) {
            it->second->destroy();
            m_framebuffers.erase(it);
        }
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::bind_buffer(RenderHandle bufferHandle,
                                                                                  BufferType type) {
        if (bufferHandle == InvalidHandle) {
            glBindBuffer(to_gl_buffer_type(type), 0);
            return ribble::core::Ok();
        }

        auto it = m_buffers.find(bufferHandle);
        if (it == m_buffers.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid buffer handle"));
        }

        it->second->bind();
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::bind_vertex_array(RenderHandle vertexArrayHandle) {
        if (vertexArrayHandle == InvalidHandle) {
            glBindVertexArray(0);
            m_state.bind_vao(0);
            return ribble::core::Ok();
        }

        auto it = m_vertexArrays.find(vertexArrayHandle);
        if (it == m_vertexArrays.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid vertex array handle"));
        }

        it->second->bind();
        m_state.bind_vao(it->second->id());
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::bind_framebuffer(RenderHandle framebufferHandle) {
        if (framebufferHandle == InvalidHandle) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_state.bind_framebuffer(GL_FRAMEBUFFER, 0);
            return ribble::core::Ok();
        }

        auto it = m_framebuffers.find(framebufferHandle);
        if (it == m_framebuffers.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid framebuffer handle"));
        }

        it->second->bind();
        m_state.bind_framebuffer(GL_FRAMEBUFFER, it->second->id());
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::draw_indexed(RenderHandle vertexArrayHandle, uint32_t indexCount, IndexType indexType,
                                uint32_t indexOffset, int32_t baseVertex, PrimitiveTopology topology) {
        auto it = m_vertexArrays.find(vertexArrayHandle);
        if (it == m_vertexArrays.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid vertex array handle"));
        }

        it->second->bind();
        GLenum glType = to_gl_index_type(indexType);
        GLenum glMode = to_gl_primitive(topology);

        // Calculate byte offset
        size_t elementSize = (indexType == IndexType::UInt16) ? sizeof(uint16_t) : sizeof(uint32_t);
        GLvoid *offsetPtr = reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(indexOffset * elementSize));

        glDrawElementsBaseVertex(glMode, static_cast<GLsizei>(indexCount), glType, offsetPtr, baseVertex);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::draw_arrays(RenderHandle vertexArrayHandle,
                                                                                  uint32_t vertexCount,
                                                                                  uint32_t vertexOffset,
                                                                                  PrimitiveTopology topology) {
        auto it = m_vertexArrays.find(vertexArrayHandle);
        if (it == m_vertexArrays.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid vertex array handle"));
        }

        it->second->bind();
        GLenum glMode = to_gl_primitive(topology);
        glDrawArrays(glMode, static_cast<GLint>(vertexOffset), static_cast<GLsizei>(vertexCount));
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    OpenGLBackend::draw_instanced(RenderHandle vertexArrayHandle, uint32_t indexCount, uint32_t instanceCount,
                                  IndexType indexType, uint32_t indexOffset, int32_t baseVertex,
                                  PrimitiveTopology topology) {
        auto it = m_vertexArrays.find(vertexArrayHandle);
        if (it == m_vertexArrays.end()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid vertex array handle"));
        }

        it->second->bind();
        GLenum glType = to_gl_index_type(indexType);
        GLenum glMode = to_gl_primitive(topology);

        if (indexCount > 0) {
            // Indexed instanced drawing
            size_t elementSize = (indexType == IndexType::UInt16) ? sizeof(uint16_t) : sizeof(uint32_t);
            GLvoid *offsetPtr = reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(indexOffset * elementSize));
            glDrawElementsInstancedBaseVertex(glMode, static_cast<GLsizei>(indexCount), glType, offsetPtr,
                                              static_cast<GLsizei>(instanceCount), baseVertex);
        } else {
            // Non-indexed instanced drawing - would need vertex count parameter
            // For now, this is a limitation - we'd need to track vertex count per VAO
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::DrawFailure, "Non-indexed instanced drawing requires vertex count"));
        }

        return ribble::core::Ok();
    }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    OpenGLBackend::create_pipeline(RenderHandle shaderHandle) {
        // For OpenGL, a "pipeline" is just a shader program
        // In Vulkan, this would create a VkPipeline with all state
        // For OpenGL, we just return the shader handle as the pipeline handle
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Invalid shader handle for pipeline creation"));
        }

        // In OpenGL, pipeline == shader program
        // We'll use the shader handle as the pipeline handle
        RenderHandle pipelineHandle = m_nextPipelineHandle++;
        m_pipelines[pipelineHandle] = shaderHandle;
        return ribble::core::Ok(pipelineHandle);
    }

    void OpenGLBackend::destroy_pipeline(RenderHandle handle) {
        m_pipelines.erase(handle);
        // Note: We don't destroy the shader, as it might be used by other pipelines
    }

    // Render state methods
    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_depth_test(bool enabled) {
        m_state.set_depth_test(enabled);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_depth_write(bool enabled) {
        m_state.set_depth_write(enabled);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_depth_func(DepthFunc func) {
        m_state.set_depth_func(func);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_blend(bool enabled) {
        m_state.set_blend(enabled);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_blend_func(BlendFactor src, BlendFactor dst) {
        m_state.set_blend_func(src, dst);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_blend_op(BlendOp op) {
        m_state.set_blend_equation(op);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_cull_mode(CullMode mode) {
        m_state.set_cull_face(mode);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_winding_order(WindingOrder order) {
        m_state.set_winding_order(order);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> OpenGLBackend::set_program_point_size(bool enabled) {
        m_state.set_program_point_size(enabled);
        return ribble::core::Ok();
    }

} // namespace backend
