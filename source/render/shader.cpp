#include "ribble/render/shader.h"
#include <algorithm>
#include "../../backend/common/render_backend.h"
#include "../../backend/common/shader_source.h"
#include "ribble/core/logger.h"

namespace ribble::render {

    Shader::Shader(backend::RenderBackend &backend, backend::RenderHandle handle, const std::string &name) :
        m_backend(backend), m_handle(handle), m_name(name) {}

    Shader::~Shader() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_shader(m_handle);
        }
    }

    Shader::Shader(Shader &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_name(std::move(other.m_name)) {
        other.m_handle = backend::InvalidHandle;
    }

    Shader &Shader::operator=(Shader &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_shader(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_name = std::move(other.m_name);
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<Shader>, ShaderFailure>
    Shader::create(backend::RenderBackend &backend, const backend::ShaderSource &source, const std::string &name) {

        auto result = backend.create_shader(source);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(ShaderFailure::CreationFailure, "Failed to create shader: {}",
                                           result.error().message));
        }

        auto shader = std::unique_ptr<Shader>(new Shader(backend, *result, name));
        RIBBLE_LOG_INFO("Created shader '{}' (handle: {})", name.empty() ? "<unnamed>" : name, *result);
        return core::Result<std::unique_ptr<Shader>, ShaderFailure>(std::in_place, std::move(shader));
    }

    core::Result<std::unique_ptr<Shader>, ShaderFailure>
    Shader::create(backend::RenderBackend &backend, const ParsedShaderSource &parsed, const std::string &name) {

        if (parsed.shaders.empty()) {
            return core::Fail(RIBBLE_ERROR(ShaderFailure::CreationFailure, "Parsed shader source is empty"));
        }

        // For multi-stage shaders, we need to create them stage by stage
        // The backend's create_shader() should handle compiling and linking multiple stages
        // For now, we'll create shaders for each stage and the backend should link them
        // This is a simplified approach - a full implementation would need backend support
        // for multi-stage shader creation

        // Use the first stage as the primary shader (typically Vertex)
        // In a full implementation, we'd need the backend to support multi-stage shader programs
        const auto &firstStage = *parsed.shaders.begin();
        backend::ShaderSource source(backend::ShaderLanguage::GLSL, firstStage.first, firstStage.second);

        // TODO: Support multi-stage shaders properly
        // For now, create a shader from the first stage
        // The backend should ideally support creating a shader program from multiple stages
        return create(backend, source, name);
    }

    core::Result<std::unique_ptr<Shader>, ShaderFailure>
    Shader::create_from_file(backend::RenderBackend &backend, const std::string &filepath, const std::string &name) {

        auto parsed = ShaderParser::parse_file(filepath);
        if (!parsed) {
            return core::Fail(
                    RIBBLE_ERROR(ShaderFailure::CompilationFailure, "Failed to parse shader file: {}", filepath));
        }

        return create(backend, *parsed, name.empty() ? filepath : name);
    }

    void Shader::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_pipeline(m_handle);
        }
    }

    void Shader::unbind() const { m_backend.bind_pipeline(backend::InvalidHandle); }

    void Shader::set_uniform(const std::string &name, int value) { m_backend.set_uniform(m_handle, name, value); }

    void Shader::set_uniform(const std::string &name, float value) { m_backend.set_uniform(m_handle, name, value); }

    void Shader::set_uniform(const std::string &name, const glm::vec2 &value) {
        m_backend.set_uniform(m_handle, name, value.x, value.y);
    }

    void Shader::set_uniform(const std::string &name, const glm::vec3 &value) {
        m_backend.set_uniform(m_handle, name, value.x, value.y, value.z);
    }

    void Shader::set_uniform(const std::string &name, const glm::vec4 &value) {
        m_backend.set_uniform(m_handle, name, value.x, value.y, value.z, value.w);
    }

    void Shader::set_uniform(const std::string &name, const glm::mat3 &value) {
        m_backend.set_uniform(m_handle, name, &value[0][0], false);
    }

    void Shader::set_uniform(const std::string &name, const glm::mat4 &value) {
        m_backend.set_uniform(m_handle, name, &value[0][0], false);
    }

    void Shader::set_uniform(const std::string &name, bool value) {
        m_backend.set_uniform(m_handle, name, value ? 1 : 0);
    }

    void Shader::set_uniform(const std::string &name, const int *values, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            m_backend.set_uniform(m_handle, name + "[" + std::to_string(i) + "]", values[i]);
        }
    }

    void Shader::set_uniform(const std::string &name, const float *values, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            m_backend.set_uniform(m_handle, name + "[" + std::to_string(i) + "]", values[i]);
        }
    }

    void Shader::set_uniform(const std::string &name, const glm::mat4 *matrices, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            m_backend.set_uniform(m_handle, name + "[" + std::to_string(i) + "]", &matrices[i][0][0], false);
        }
    }

} // namespace ribble::render
