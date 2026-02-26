#pragma once
#include <glad/gl.h>
#include <ribble/core/fail.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../../common/shader_source.h"
#include "backend_types.h"

namespace backend {

    class OpenGLShader {
    public:
        enum class Failure {
            CompilationFailure,
            LinkFailure,
            InvalidUniform,
        };

        OpenGLShader() = default;
        ~OpenGLShader();

        OpenGLShader(const OpenGLShader &) = delete;
        OpenGLShader &operator=(const OpenGLShader &) = delete;
        OpenGLShader(OpenGLShader &&) noexcept;
        OpenGLShader &operator=(OpenGLShader &&) noexcept;

        /// Compile a shader stage from source
        ribble::core::Result<void, Failure> compile(ShaderStage stage, std::string_view source);

        /// Compile from ShaderSource (language-agnostic, but expects GLSL for OpenGL)
        ribble::core::Result<void, Failure> compile(const ShaderSource &source);

        /// Link all compiled shader stages into a program
        ribble::core::Result<void, Failure> link();

        // Convenience: compile both stages and link in one call
        ribble::core::Result<void, Failure> build(std::string_view vertexSrc, std::string_view fragmentSrc);

        void bind() const;
        void unbind() const;
        void destroy();

        [[nodiscard]] GLuint program_id() const { return m_program; }
        [[nodiscard]] bool is_valid() const { return m_program != 0; }

        // ── Uniforms ──────────────────────────────────────────────────────────
        void set_int(std::string_view name, int value);
        void set_float(std::string_view name, float value);
        void set_vec2(std::string_view name, float x, float y);
        void set_vec3(std::string_view name, float x, float y, float z);
        void set_vec4(std::string_view name, float x, float y, float z, float w);
        void set_mat3(std::string_view name, const float *data, bool transpose = false);
        void set_mat4(std::string_view name, const float *data, bool transpose = false);
        void set_bool(std::string_view name, bool value);

        // Array uniforms
        void set_int_array(std::string_view name, const int *values, size_t count);
        void set_float_array(std::string_view name, const float *values, size_t count);
        void set_mat4_array(std::string_view name, const float *matrices, size_t count, bool transpose = false);

    private:
        [[nodiscard]] GLint uniform_location(std::string_view name);
        static std::string shader_stage_to_string(ShaderStage stage);

        GLuint m_program{0};
        std::vector<GLuint> m_compiledShaders; // Track compiled shaders for linking
        std::unordered_map<std::string, GLint> m_uniformCache;
    };

} // namespace backend

RIBBLE_ENUM_TO_STRING(backend::OpenGLShader::Failure,
                      case backend::OpenGLShader::Failure::CompilationFailure : return "Shader Compilation Failure";
                      case backend::OpenGLShader::Failure::LinkFailure : return "Shader Link Failure";
                      case backend::OpenGLShader::Failure::InvalidUniform : return "Invalid Uniform";);
