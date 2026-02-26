#include "opengl_buffer.h"
#include "opengl_conversions.h"

namespace backend {

    // ── OpenGLBuffer ──────────────────────────────────────────────────────────

    OpenGLBuffer::~OpenGLBuffer() { destroy(); }

    OpenGLBuffer::OpenGLBuffer(OpenGLBuffer &&o) noexcept :
        m_id(o.m_id), m_target(o.m_target), m_usage(o.m_usage), m_sizeBytes(o.m_sizeBytes) {
        o.m_id = 0;
    }

    OpenGLBuffer &OpenGLBuffer::operator=(OpenGLBuffer &&o) noexcept {
        if (this != &o) {
            destroy();
            m_id = o.m_id;
            m_target = o.m_target;
            m_usage = o.m_usage;
            m_sizeBytes = o.m_sizeBytes;
            o.m_id = 0;
        }
        return *this;
    }

    ribble::core::Result<void, OpenGLBuffer::Failure> OpenGLBuffer::create(BufferType type, BufferUsage usage,
                                                                           const void *data, size_t sizeBytes) {
        destroy();
        m_target = to_gl_buffer_type(type);
        m_usage = to_gl_buffer_usage(usage);
        m_sizeBytes = sizeBytes;

        glGenBuffers(1, &m_id);
        if (!m_id)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::CreationFailure, "glGenBuffers failed"));

        glBindBuffer(m_target, m_id);
        glBufferData(m_target, static_cast<GLsizeiptr>(sizeBytes), data, m_usage);
        glBindBuffer(m_target, 0);
        return ribble::core::Ok();
    }

    void OpenGLBuffer::upload(const void *data, size_t sizeBytes) {
        m_sizeBytes = sizeBytes;
        glBindBuffer(m_target, m_id);
        glBufferData(m_target, static_cast<GLsizeiptr>(sizeBytes), data, m_usage);
        glBindBuffer(m_target, 0);
    }

    void OpenGLBuffer::upload_sub(const void *data, size_t sizeBytes, size_t offsetBytes) {
        glBindBuffer(m_target, m_id);
        glBufferSubData(m_target, static_cast<GLintptr>(offsetBytes), static_cast<GLsizeiptr>(sizeBytes), data);
        glBindBuffer(m_target, 0);
    }

    void OpenGLBuffer::bind() const { glBindBuffer(m_target, m_id); }
    void OpenGLBuffer::unbind() const { glBindBuffer(m_target, 0); }

    void OpenGLBuffer::destroy() {
        if (m_id) {
            glDeleteBuffers(1, &m_id);
            m_id = 0;
        }
    }

    // ── OpenGLVertexArray ─────────────────────────────────────────────────────

    OpenGLVertexArray::~OpenGLVertexArray() { destroy(); }

    OpenGLVertexArray::OpenGLVertexArray(OpenGLVertexArray &&o) noexcept : m_id(o.m_id) { o.m_id = 0; }

    OpenGLVertexArray &OpenGLVertexArray::operator=(OpenGLVertexArray &&o) noexcept {
        if (this != &o) {
            destroy();
            m_id = o.m_id;
            o.m_id = 0;
        }
        return *this;
    }

    void OpenGLVertexArray::create() {
        destroy();
        glGenVertexArrays(1, &m_id);
    }

    void OpenGLVertexArray::set_attribute(GLuint attribIndex, GLint components, GLenum glType, bool normalized,
                                          GLsizei stride, size_t offset) {
        bind();
        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex, components, glType, normalized ? GL_TRUE : GL_FALSE, stride,
                              reinterpret_cast<const void *>(offset));
        unbind();
    }

    void OpenGLVertexArray::bind() const { glBindVertexArray(m_id); }
    void OpenGLVertexArray::unbind() const { glBindVertexArray(0); }

    void OpenGLVertexArray::destroy() {
        if (m_id) {
            glDeleteVertexArrays(1, &m_id);
            m_id = 0;
        }
    }

} // namespace backend
