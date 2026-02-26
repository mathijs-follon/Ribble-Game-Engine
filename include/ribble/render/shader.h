#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "ribble/core/fail.h"
#include "ribble/render/parsed_shader_source.h"
#include "ribble/render/shader_parser.h"

// Forward declarations
namespace backend {
    class RenderBackend;
    struct ShaderSource;
    using RenderHandle = uint32_t;
} // namespace backend

namespace ribble::render {

    enum class ShaderFailure {
        CreationFailure,
        CompilationFailure,
        InvalidShader,
    };

    /// High-level, backend-agnostic Shader class
    /// Wraps backend shader handles and provides convenient interface
    class Shader {
    public:
        /// Create shader from ShaderSource (language-agnostic)
        /// @param backend Render backend to create shader with
        /// @param source Shader source (GLSL, HLSL, MSL, or SPIR-V)
        /// @param name Optional name for debugging
        static core::Result<std::unique_ptr<Shader>, ShaderFailure>
        create(backend::RenderBackend &backend, const backend::ShaderSource &source, const std::string &name = "");

        /// Create shader from ParsedShaderSource (multiple stages from GLSL parser)
        /// @param backend Render backend to create shader with
        /// @param parsed Parsed shader source from ShaderParser
        /// @param name Optional name for debugging
        static core::Result<std::unique_ptr<Shader>, ShaderFailure>
        create(backend::RenderBackend &backend, const ParsedShaderSource &parsed, const std::string &name = "");

        /// Create shader from GLSL file (convenience method)
        /// @param backend Render backend to create shader with
        /// @param filepath Path to GLSL shader file
        /// @param name Optional name for debugging
        static core::Result<std::unique_ptr<Shader>, ShaderFailure>
        create_from_file(backend::RenderBackend &backend, const std::string &filepath, const std::string &name = "");

        ~Shader();

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;

        /// Bind this shader for rendering
        void bind() const;

        /// Unbind shader (bind null shader)
        void unbind() const;

        /// Get the backend shader handle
        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }

        /// Check if shader is valid
        [[nodiscard]] bool is_valid() const { return m_handle != ::backend::InvalidHandle; }

        /// Get shader name
        [[nodiscard]] const std::string &name() const { return m_name; }

        // ── Uniform Setting ────────────────────────────────────────────────────

        void set_uniform(const std::string &name, int value);
        void set_uniform(const std::string &name, float value);
        void set_uniform(const std::string &name, const glm::vec2 &value);
        void set_uniform(const std::string &name, const glm::vec3 &value);
        void set_uniform(const std::string &name, const glm::vec4 &value);
        void set_uniform(const std::string &name, const glm::mat3 &value);
        void set_uniform(const std::string &name, const glm::mat4 &value);
        void set_uniform(const std::string &name, bool value);

        // Array uniforms
        void set_uniform(const std::string &name, const int *values, size_t count);
        void set_uniform(const std::string &name, const float *values, size_t count);
        void set_uniform(const std::string &name, const glm::mat4 *matrices, size_t count);

    private:
        Shader(backend::RenderBackend &backend, backend::RenderHandle handle, const std::string &name);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{::backend::InvalidHandle};
        std::string m_name;
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(ribble::render::ShaderFailure,
                      case ribble::render::ShaderFailure::CreationFailure : return "Shader Creation Failure";
                      case ribble::render::ShaderFailure::CompilationFailure : return "Shader Compilation Failure";
                      case ribble::render::ShaderFailure::InvalidShader : return "Invalid Shader";);
