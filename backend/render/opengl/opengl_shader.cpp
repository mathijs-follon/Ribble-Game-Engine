#include "opengl_shader.h"
#include <ribble/core/logger.h>
#include <vector>
#include "opengl_conversions.h"

namespace ribble::backend::opengl {

    OpenGLShader::~OpenGLShader() { destroy(); }

    OpenGLShader::OpenGLShader(OpenGLShader &&other) noexcept :
        m_program(other.m_program), m_uniformCache(std::move(other.m_uniformCache)) {
        other.m_program = 0;
    }

    OpenGLShader &OpenGLShader::operator=(OpenGLShader &&other) noexcept {
        if (this != &other) {
            destroy();
            m_program = other.m_program;
            m_uniformCache = std::move(other.m_uniformCache);
            other.m_program = 0;
        }
        return *this;
    }

    core::Result<void, OpenGLShader::Failure> OpenGLShader::compile(ShaderStage stage, std::string_view source) {
        const GLenum glStage = to_gl_shader_stage(stage);
        const GLuint shader = glCreateShader(glStage);

        const char *src = source.data();
        const GLint len = static_cast<GLint>(source.size());
        glShaderSource(shader, 1, &src, &len);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());
            glDeleteShader(shader);
            return core::Fail(RIBBLE_ERROR(Failure::CompilationFailure, "Shader compilation failed:\n{}", log.data()));
        }

        // Attach to program, creating it on first compile
        if (!m_program)
            m_program = glCreateProgram();
        glAttachShader(m_program, shader);
        glDeleteShader(shader); // Shader is safe to delete after attach
        return core::Ok();
    }

    core::Result<void, OpenGLShader::Failure> OpenGLShader::link(std::initializer_list<GLuint> shaderIds) {
        if (!m_program)
            m_program = glCreateProgram();

        for (GLuint id: shaderIds)
            glAttachShader(m_program, id);

        glLinkProgram(m_program);

        GLint success = 0;
        glGetProgramiv(m_program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLen = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
            return core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "Shader link failed:\n{}", log.data()));
        }

        m_uniformCache.clear();
        return core::Ok();
    }

    core::Result<void, OpenGLShader::Failure> OpenGLShader::build(std::string_view vertexSrc,
                                                                  std::string_view fragmentSrc) {
        m_program = glCreateProgram();
        m_uniformCache.clear();

        auto compile_stage = [&](GLenum type, std::string_view src) -> core::Result<GLuint, Failure> {
            GLuint shader = glCreateShader(type);
            const char *s = src.data();
            const GLint l = static_cast<GLint>(src.size());
            glShaderSource(shader, 1, &s, &l);
            glCompileShader(shader);
            GLint ok = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                GLint logLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
                std::vector<char> log(logLen);
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
                glDeleteShader(shader);
                return core::Fail(
                        RIBBLE_ERROR(Failure::CompilationFailure, "Shader compilation failed:\n{}", log.data()));
            }
            return core::Ok(shader);
        };

        auto vert = compile_stage(GL_VERTEX_SHADER, vertexSrc);
        if (!vert)
            return core::Fail(vert.error());
        auto frag = compile_stage(GL_FRAGMENT_SHADER, fragmentSrc);
        if (!frag) {
            glDeleteShader(*vert);
            return core::Fail(frag.error());
        }

        glAttachShader(m_program, *vert);
        glAttachShader(m_program, *frag);
        glLinkProgram(m_program);
        glDeleteShader(*vert);
        glDeleteShader(*frag);

        GLint ok = 0;
        glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint logLen = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
            destroy();
            return core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "Shader link failed:\n{}", log.data()));
        }

        return core::Ok();
    }

    void OpenGLShader::bind() const { glUseProgram(m_program); }
    void OpenGLShader::unbind() const { glUseProgram(0); }

    void OpenGLShader::destroy() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        m_uniformCache.clear();
    }

    GLint OpenGLShader::uniform_location(std::string_view name) {
        auto key = std::string(name);
        auto it = m_uniformCache.find(key);
        if (it != m_uniformCache.end())
            return it->second;
        GLint loc = glGetUniformLocation(m_program, key.c_str());
        if (loc == -1)
            RIBBLE_LOG_WARNING("Uniform '{}' not found in shader program {}", name, m_program);
        m_uniformCache.emplace(std::move(key), loc);
        return loc;
    }

    void OpenGLShader::set_int(std::string_view n, int v) { glUniform1i(uniform_location(n), v); }
    void OpenGLShader::set_float(std::string_view n, float v) { glUniform1f(uniform_location(n), v); }
    void OpenGLShader::set_vec2(std::string_view n, float x, float y) { glUniform2f(uniform_location(n), x, y); }
    void OpenGLShader::set_vec3(std::string_view n, float x, float y, float z) {
        glUniform3f(uniform_location(n), x, y, z);
    }
    void OpenGLShader::set_vec4(std::string_view n, float x, float y, float z, float w) {
        glUniform4f(uniform_location(n), x, y, z, w);
    }
    void OpenGLShader::set_mat3(std::string_view n, const float *d, bool t) {
        glUniformMatrix3fv(uniform_location(n), 1, t, d);
    }
    void OpenGLShader::set_mat4(std::string_view n, const float *d, bool t) {
        glUniformMatrix4fv(uniform_location(n), 1, t, d);
    }
    void OpenGLShader::set_bool(std::string_view n, bool v) { glUniform1i(uniform_location(n), v ? 1 : 0); }

} // namespace ribble::backend::opengl
