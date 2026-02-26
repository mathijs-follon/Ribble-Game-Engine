#pragma once
#include <glad/gl.h>
#include <ribble/core/fail.h>
#include "backend_types.h"

namespace ribble::backend::opengl {

    class OpenGLBuffer {
    public:
        enum class Failure {
            CreationFailure,
            InvalidBuffer,
        };

        OpenGLBuffer() = default;
        ~OpenGLBuffer();

        OpenGLBuffer(const OpenGLBuffer &) = delete;
        OpenGLBuffer &operator=(const OpenGLBuffer &) = delete;
        OpenGLBuffer(OpenGLBuffer &&) noexcept;
        OpenGLBuffer &operator=(OpenGLBuffer &&) noexcept;

        core::Result<void, Failure> create(BufferType type, BufferUsage usage, const void *data, size_t sizeBytes);

        /// Resize or replace all data (re-allocates GPU buffer)
        void upload(const void *data, size_t sizeBytes);

        /// Update a sub-range (must fit within current allocation)
        void upload_sub(const void *data, size_t sizeBytes, size_t offsetBytes = 0);

        void bind() const;
        void unbind() const;
        void destroy();

        [[nodiscard]] GLuint id() const { return m_id; }
        [[nodiscard]] GLenum target() const { return m_target; }
        [[nodiscard]] GLenum usage() const { return m_usage; }
        [[nodiscard]] size_t size_bytes() const { return m_sizeBytes; }
        [[nodiscard]] bool is_valid() const { return m_id != 0; }

    private:
        GLuint m_id{0};
        GLenum m_target{GL_ARRAY_BUFFER};
        GLenum m_usage{GL_STATIC_DRAW};
        size_t m_sizeBytes{0};
    };

    /// Vertex Array Object — owns the vertex attribute layout
    class OpenGLVertexArray {
    public:
        OpenGLVertexArray() = default;
        ~OpenGLVertexArray();

        OpenGLVertexArray(const OpenGLVertexArray &) = delete;
        OpenGLVertexArray &operator=(const OpenGLVertexArray &) = delete;
        OpenGLVertexArray(OpenGLVertexArray &&) noexcept;
        OpenGLVertexArray &operator=(OpenGLVertexArray &&) noexcept;

        void create();
        void destroy();

        /// Bind a VBO and describe a vertex attribute.
        /// @param attribIndex  Location in shader (layout location = N)
        /// @param components   Number of components (1-4)
        /// @param glType       GL_FLOAT, GL_INT, etc.
        /// @param normalized   Normalize integer types?
        /// @param stride       Byte stride between vertices
        /// @param offset       Byte offset of this attribute within a vertex
        void set_attribute(GLuint attribIndex, GLint components, GLenum glType, bool normalized, GLsizei stride,
                           size_t offset);

        void bind() const;
        void unbind() const;

        [[nodiscard]] GLuint id() const { return m_id; }
        [[nodiscard]] bool is_valid() const { return m_id != 0; }

    private:
        GLuint m_id{0};
    };

} // namespace ribble::backend::opengl
