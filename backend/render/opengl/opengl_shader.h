#pragma once
#include <glad/gl.h>
#include <ribble/core/fail.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include "backend_types.h"

namespace ribble::backend::opengl {

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

        core::Result<void, Failure> compile(ShaderStage stage, std::string_view source);
        core::Result<void, Failure> link(std::initializer_list<GLuint> shaderIds);

        // Convenience: compile both stages and link in one call
        core::Result<void, Failure> build(std::string_view vertexSrc, std::string_view fragmentSrc);

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

    private:
        [[nodiscard]] GLint uniform_location(std::string_view name);

        GLuint m_program{0};
        std::unordered_map<std::string, GLint> m_uniformCache;
    };

} // namespace ribble::backend::opengl

RIBBLE_ENUM_TO_STRING(
        ribble::backend::opengl::OpenGLShader::Failure,
        case ribble::backend::opengl::OpenGLShader::Failure::CompilationFailure : return "Shader Compilation Failure";
        case ribble::backend::opengl::OpenGLShader::Failure::LinkFailure : return "Shader Link Failure";
        case ribble::backend::opengl::OpenGLShader::Failure::InvalidUniform : return "Invalid Uniform";);
