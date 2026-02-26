#pragma once
#include <memory>
#include <vector>
#include "../../backend/common/backend_types.h"
#include "../../backend/common/render_backend.h"
#include "ribble/core/fail.h"

namespace ribble::render {

    enum class BufferFailure {
        CreationFailure,
        InvalidBuffer,
    };

    /// High-level VertexBuffer class
    class VertexBuffer {
    public:
        static core::Result<std::unique_ptr<VertexBuffer>, BufferFailure>
        create(backend::RenderBackend &backend, const void *data, size_t size,
               backend::BufferUsage usage = backend::BufferUsage::Static);

        ~VertexBuffer();

        VertexBuffer(const VertexBuffer &) = delete;
        VertexBuffer &operator=(const VertexBuffer &) = delete;
        VertexBuffer(VertexBuffer &&other) noexcept;
        VertexBuffer &operator=(VertexBuffer &&other) noexcept;

        void bind() const;
        void unbind() const;

        void set_data(const void *data, size_t size);
        void update_data(const void *data, size_t size, size_t offset = 0);

        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }
        [[nodiscard]] size_t size() const { return m_size; }
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

    private:
        VertexBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t size);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        size_t m_size{0};
    };

    /// High-level IndexBuffer class
    class IndexBuffer {
    public:
        static core::Result<std::unique_ptr<IndexBuffer>, BufferFailure>
        create(backend::RenderBackend &backend, const void *data, size_t count,
               backend::IndexType indexType = backend::IndexType::UInt32,
               backend::BufferUsage usage = backend::BufferUsage::Static);

        ~IndexBuffer();

        IndexBuffer(const IndexBuffer &) = delete;
        IndexBuffer &operator=(const IndexBuffer &) = delete;
        IndexBuffer(IndexBuffer &&other) noexcept;
        IndexBuffer &operator=(IndexBuffer &&other) noexcept;

        void bind() const;
        void unbind() const;

        void set_data(const void *data, size_t count);
        void update_data(const void *data, size_t count, size_t offset = 0);

        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }
        [[nodiscard]] size_t count() const { return m_count; }
        [[nodiscard]] backend::IndexType index_type() const { return m_indexType; }
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

    private:
        IndexBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t count,
                    backend::IndexType indexType);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        size_t m_count{0};
        backend::IndexType m_indexType{backend::IndexType::UInt32};
    };

    /// High-level UniformBuffer class
    class UniformBuffer {
    public:
        static core::Result<std::unique_ptr<UniformBuffer>, BufferFailure>
        create(backend::RenderBackend &backend, const void *data, size_t size,
               backend::BufferUsage usage = backend::BufferUsage::Dynamic);

        ~UniformBuffer();

        UniformBuffer(const UniformBuffer &) = delete;
        UniformBuffer &operator=(const UniformBuffer &) = delete;
        UniformBuffer(UniformBuffer &&other) noexcept;
        UniformBuffer &operator=(UniformBuffer &&other) noexcept;

        void bind() const;
        void bind_to_binding_point(uint32_t bindingPoint) const;

        void set_data(const void *data, size_t size);
        void update_data(const void *data, size_t size, size_t offset = 0);

        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }
        [[nodiscard]] size_t size() const { return m_size; }
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

    private:
        UniformBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t size);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        size_t m_size{0};
    };

    /// Vertex attribute descriptor
    struct VertexAttribute {
        uint32_t index;
        int32_t size; // Number of components (1-4)
        uint32_t type; // GL_FLOAT, GL_INT, etc. (backend-specific)
        bool normalized; // Normalize integer types?
        size_t offset; // Offset in bytes
        size_t stride; // Stride in bytes
    };

    /// High-level VertexArray class
    class VertexArray {
    public:
        static core::Result<std::unique_ptr<VertexArray>, BufferFailure> create(backend::RenderBackend &backend);

        ~VertexArray();

        VertexArray(const VertexArray &) = delete;
        VertexArray &operator=(const VertexArray &) = delete;
        VertexArray(VertexArray &&other) noexcept;
        VertexArray &operator=(VertexArray &&other) noexcept;

        void bind() const;
        void unbind() const;

        void add_vertex_buffer(std::shared_ptr<VertexBuffer> buffer);
        void set_index_buffer(std::shared_ptr<IndexBuffer> buffer);

        void set_vertex_attribute(const VertexAttribute &attrib);
        void enable_vertex_attribute(uint32_t index);
        void disable_vertex_attribute(uint32_t index);

        void set_vertex_count(size_t count) { m_vertexCount = count; }
        [[nodiscard]] size_t vertex_count() const { return m_vertexCount; }

        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

    private:
        VertexArray(backend::RenderBackend &backend, backend::RenderHandle handle);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
        std::shared_ptr<IndexBuffer> m_indexBuffer;
        size_t m_vertexCount{0};
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(ribble::render::BufferFailure,
                      case ribble::render::BufferFailure::CreationFailure : return "Buffer Creation Failure";
                      case ribble::render::BufferFailure::InvalidBuffer : return "Invalid Buffer";);
