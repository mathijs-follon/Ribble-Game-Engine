#include "ribble/render/buffer.h"
#include <glad/gl.h>
#include "ribble/core/logger.h"

namespace ribble::render {

    // ── VertexBuffer ────────────────────────────────────────────────────────────

    VertexBuffer::VertexBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t size) :
        m_backend(backend), m_handle(handle), m_size(size) {}

    VertexBuffer::~VertexBuffer() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_buffer(m_handle);
        }
    }

    VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_size(other.m_size) {
        other.m_handle = backend::InvalidHandle;
    }

    VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_buffer(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_size = other.m_size;
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<VertexBuffer>, BufferFailure>
    VertexBuffer::create(backend::RenderBackend &backend, const void *data, size_t size, backend::BufferUsage usage) {

        auto result = backend.create_buffer(backend::BufferType::Vertex, usage, size, data);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(BufferFailure::CreationFailure, "Failed to create vertex buffer"));
        }

        auto buffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(backend, *result, size));
        return core::Result<std::unique_ptr<VertexBuffer>, BufferFailure>(std::in_place, std::move(buffer));
    }

    void VertexBuffer::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_buffer(m_handle, backend::BufferType::Vertex);
        }
    }

    void VertexBuffer::unbind() const { m_backend.bind_buffer(backend::InvalidHandle, backend::BufferType::Vertex); }

    void VertexBuffer::set_data(const void *data, size_t size) {
        // This would need backend support for buffer updates
        // For now, this is a placeholder
        // TODO: Add update_buffer_data to RenderBackend interface
        m_size = size;
    }

    void VertexBuffer::update_data(const void *data, size_t size, size_t offset) {
        // This would need backend support for buffer sub-updates
        // For now, this is a placeholder
        // TODO: Add update_buffer_sub_data to RenderBackend interface
    }

    // ── IndexBuffer ──────────────────────────────────────────────────────────────

    IndexBuffer::IndexBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t count,
                             backend::IndexType indexType) :
        m_backend(backend), m_handle(handle), m_count(count), m_indexType(indexType) {}

    IndexBuffer::~IndexBuffer() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_buffer(m_handle);
        }
    }

    IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_count(other.m_count), m_indexType(other.m_indexType) {
        other.m_handle = backend::InvalidHandle;
    }

    IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_buffer(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_count = other.m_count;
            m_indexType = other.m_indexType;
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<IndexBuffer>, BufferFailure> IndexBuffer::create(backend::RenderBackend &backend,
                                                                                  const void *data, size_t count,
                                                                                  backend::IndexType indexType,
                                                                                  backend::BufferUsage usage) {

        size_t elementSize = (indexType == backend::IndexType::UInt16) ? sizeof(uint16_t) : sizeof(uint32_t);
        size_t sizeBytes = count * elementSize;

        auto result = backend.create_buffer(backend::BufferType::Index, usage, sizeBytes, data);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(BufferFailure::CreationFailure, "Failed to create index buffer"));
        }

        auto buffer = std::unique_ptr<IndexBuffer>(new IndexBuffer(backend, *result, count, indexType));
        return core::Result<std::unique_ptr<IndexBuffer>, BufferFailure>(std::in_place, std::move(buffer));
    }

    void IndexBuffer::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_buffer(m_handle, backend::BufferType::Index);
        }
    }

    void IndexBuffer::unbind() const { m_backend.bind_buffer(backend::InvalidHandle, backend::BufferType::Index); }

    void IndexBuffer::set_data(const void *data, size_t count) {
        // Placeholder - would need backend support
        m_count = count;
    }

    void IndexBuffer::update_data(const void *data, size_t count, size_t offset) {
        // Placeholder - would need backend support
    }

    // ── UniformBuffer ────────────────────────────────────────────────────────────

    UniformBuffer::UniformBuffer(backend::RenderBackend &backend, backend::RenderHandle handle, size_t size) :
        m_backend(backend), m_handle(handle), m_size(size) {}

    UniformBuffer::~UniformBuffer() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_buffer(m_handle);
        }
    }

    UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_size(other.m_size) {
        other.m_handle = backend::InvalidHandle;
    }

    UniformBuffer &UniformBuffer::operator=(UniformBuffer &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_buffer(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_size = other.m_size;
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<UniformBuffer>, BufferFailure>
    UniformBuffer::create(backend::RenderBackend &backend, const void *data, size_t size, backend::BufferUsage usage) {

        auto result = backend.create_buffer(backend::BufferType::Uniform, usage, size, data);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(BufferFailure::CreationFailure, "Failed to create uniform buffer"));
        }

        auto buffer = std::unique_ptr<UniformBuffer>(new UniformBuffer(backend, *result, size));
        return core::Result<std::unique_ptr<UniformBuffer>, BufferFailure>(std::in_place, std::move(buffer));
    }

    void UniformBuffer::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_buffer(m_handle, backend::BufferType::Uniform);
        }
    }

    void UniformBuffer::bind_to_binding_point(uint32_t bindingPoint) const {
        bind();
        // For OpenGL, we'd use glBindBufferBase
        // This would need backend support
        // TODO: Add bind_buffer_to_binding_point to RenderBackend interface
    }

    void UniformBuffer::set_data(const void *data, size_t size) {
        // Placeholder - would need backend support
        m_size = size;
    }

    void UniformBuffer::update_data(const void *data, size_t size, size_t offset) {
        // Placeholder - would need backend support
    }

    // ── VertexArray ─────────────────────────────────────────────────────────────

    VertexArray::VertexArray(backend::RenderBackend &backend, backend::RenderHandle handle) :
        m_backend(backend), m_handle(handle) {}

    VertexArray::~VertexArray() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_vertex_array(m_handle);
        }
    }

    VertexArray::VertexArray(VertexArray &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_vertexBuffers(std::move(other.m_vertexBuffers)),
        m_indexBuffer(std::move(other.m_indexBuffer)), m_vertexCount(other.m_vertexCount) {
        other.m_handle = backend::InvalidHandle;
    }

    VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_vertex_array(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_vertexBuffers = std::move(other.m_vertexBuffers);
            m_indexBuffer = std::move(other.m_indexBuffer);
            m_vertexCount = other.m_vertexCount;
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<VertexArray>, BufferFailure> VertexArray::create(backend::RenderBackend &backend) {
        auto result = backend.create_vertex_array();
        if (!result) {
            return core::Fail(RIBBLE_ERROR(BufferFailure::CreationFailure, "Failed to create vertex array"));
        }

        auto vertexArray = std::unique_ptr<VertexArray>(new VertexArray(backend, *result));
        return core::Result<std::unique_ptr<VertexArray>, BufferFailure>(std::in_place, std::move(vertexArray));
    }

    void VertexArray::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_vertex_array(m_handle);
        }
    }

    void VertexArray::unbind() const { m_backend.bind_vertex_array(backend::InvalidHandle); }

    void VertexArray::add_vertex_buffer(std::shared_ptr<VertexBuffer> buffer) { m_vertexBuffers.push_back(buffer); }

    void VertexArray::set_index_buffer(std::shared_ptr<IndexBuffer> buffer) { m_indexBuffer = buffer; }

    void VertexArray::set_vertex_attribute(const VertexAttribute &attrib) {
        // This would need backend support for setting vertex attributes
        // For OpenGL, we'd use glVertexAttribPointer
        // TODO: Add set_vertex_attribute to RenderBackend interface
    }

    void VertexArray::enable_vertex_attribute(uint32_t index) {
        // This would need backend support
        // TODO: Add enable_vertex_attribute to RenderBackend interface
    }

    void VertexArray::disable_vertex_attribute(uint32_t index) {
        // This would need backend support
        // TODO: Add disable_vertex_attribute to RenderBackend interface
    }

} // namespace ribble::render
