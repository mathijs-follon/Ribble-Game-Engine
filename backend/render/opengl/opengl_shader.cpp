#include "opengl_shader.h"
#include <ribble/core/logger.h>
#include <vector>
#include "opengl_conversions.h"

namespace backend {

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

    ribble::core::Result<void, OpenGLShader::Failure> OpenGLShader::compile(ShaderStage stage,
                                                                            std::string_view source) {
        const GLenum glStage = to_gl_shader_stage(stage);
        const GLuint shader = glCreateShader(glStage);
        if (shader == 0) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::CompilationFailure, "Failed to create {} shader object",
                                                   shader_stage_to_string(stage)));
        }

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
            if (logLen > 0) {
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
            }
            std::string errorMsg = "[" + shader_stage_to_string(stage) + " Shader] ";
            if (logLen > 0) {
                errorMsg += log.data();
            } else {
                errorMsg += "Compilation failed (no error log available)";
            }
            glDeleteShader(shader);
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::CompilationFailure, "Shader compilation failed:\n{}", errorMsg));
        }

        // Attach to program, creating it on first compile
        if (!m_program)
            m_program = glCreateProgram();
        if (m_program == 0) {
            glDeleteShader(shader);
            return ribble::core::Fail(RIBBLE_ERROR(Failure::CompilationFailure, "Failed to create shader program"));
        }
        glAttachShader(m_program, shader);
        m_compiledShaders.push_back(shader);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, OpenGLShader::Failure> OpenGLShader::compile(const ShaderSource &source) {
        // OpenGL only supports GLSL
        if (source.language != ShaderLanguage::GLSL) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::CompilationFailure, "OpenGL backend only supports GLSL shaders"));
        }

        std::string sourceStr = source.as_string();
        return compile(source.stage, sourceStr);
    }

    ribble::core::Result<void, OpenGLShader::Failure> OpenGLShader::link() {
        if (m_compiledShaders.empty()) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "No shaders to link"));
        }

        if (!m_program) {
            m_program = glCreateProgram();
            if (m_program == 0) {
                return ribble::core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "Failed to create shader program"));
            }
        }

        // Attach all compiled shaders
        for (GLuint shaderId: m_compiledShaders) {
            glAttachShader(m_program, shaderId);
        }

        glLinkProgram(m_program);

        GLint success = 0;
        glGetProgramiv(m_program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLen = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            std::string errorMsg;
            if (logLen > 0) {
                glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
                errorMsg = "[Link Error] " + std::string(log.data());
            } else {
                errorMsg = "Shader program linking failed (no error log available)";
            }

            // Clean up shaders
            for (GLuint shaderId: m_compiledShaders) {
                glDetachShader(m_program, shaderId);
                glDeleteShader(shaderId);
            }
            m_compiledShaders.clear();
            glDeleteProgram(m_program);
            m_program = 0;

            return ribble::core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "Shader link failed:\n{}", errorMsg));
        }

        // Clean up shader objects (they're attached to program now)
        for (GLuint shaderId: m_compiledShaders) {
            glDetachShader(m_program, shaderId);
            glDeleteShader(shaderId);
        }
        m_compiledShaders.clear();
        m_uniformCache.clear();

        RIBBLE_LOG_INFO("Successfully linked shader program (ID: {})", m_program);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, OpenGLShader::Failure> OpenGLShader::build(std::string_view vertexSrc,
                                                                          std::string_view fragmentSrc) {
        m_program = glCreateProgram();
        m_uniformCache.clear();

        auto compile_stage = [&](GLenum type, std::string_view src) -> ribble::core::Result<GLuint, Failure> {
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
                return ribble::core::Fail(
                        RIBBLE_ERROR(Failure::CompilationFailure, "Shader compilation failed:\n{}", log.data()));
            }
            return ribble::core::Ok(shader);
        };

        auto vert = compile_stage(GL_VERTEX_SHADER, vertexSrc);
        if (!vert)
            return ribble::core::Fail(vert.error());
        auto frag = compile_stage(GL_FRAGMENT_SHADER, fragmentSrc);
        if (!frag) {
            glDeleteShader(*vert);
            return ribble::core::Fail(frag.error());
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
            return ribble::core::Fail(RIBBLE_ERROR(Failure::LinkFailure, "Shader link failed:\n{}", log.data()));
        }

        return ribble::core::Ok();
    }

    void OpenGLShader::bind() const { glUseProgram(m_program); }
    void OpenGLShader::unbind() const { glUseProgram(0); }

    void OpenGLShader::destroy() {
        // Clean up compiled shaders
        for (GLuint shaderId: m_compiledShaders) {
            if (m_program) {
                glDetachShader(m_program, shaderId);
            }
            glDeleteShader(shaderId);
        }
        m_compiledShaders.clear();

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

    void OpenGLShader::set_int_array(std::string_view n, const int *values, size_t count) {
        GLint loc = uniform_location(n);
        if (loc >= 0) {
            glUniform1iv(loc, static_cast<GLsizei>(count), values);
        }
    }

    void OpenGLShader::set_float_array(std::string_view n, const float *values, size_t count) {
        GLint loc = uniform_location(n);
        if (loc >= 0) {
            glUniform1fv(loc, static_cast<GLsizei>(count), values);
        }
    }

    void OpenGLShader::set_mat4_array(std::string_view n, const float *matrices, size_t count, bool transpose) {
        GLint loc = uniform_location(n);
        if (loc >= 0) {
            glUniformMatrix4fv(loc, static_cast<GLsizei>(count), transpose ? GL_TRUE : GL_FALSE, matrices);
        }
    }

    std::string OpenGLShader::shader_stage_to_string(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return "Vertex";
            case ShaderStage::Fragment:
                return "Fragment";
            case ShaderStage::Geometry:
                return "Geometry";
            case ShaderStage::Compute:
                return "Compute";
            case ShaderStage::TessellationControl:
                return "TessellationControl";
            case ShaderStage::TessellationEvaluation:
                return "TessellationEvaluation";
            default:
                return "Unknown";
        }
    }

} // namespace backend
