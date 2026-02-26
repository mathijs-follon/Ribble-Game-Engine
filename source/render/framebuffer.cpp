#include "ribble/render/framebuffer.h"
#include "ribble/core/logger.h"
#include "ribble/render/texture.h"

namespace ribble::render {

    Framebuffer::Framebuffer(backend::RenderBackend &backend, backend::RenderHandle handle, const std::string &name) :
        m_backend(backend), m_handle(handle), m_name(name) {}

    Framebuffer::~Framebuffer() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_framebuffer(m_handle);
        }
    }

    Framebuffer::Framebuffer(Framebuffer &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_name(std::move(other.m_name)) {
        other.m_handle = backend::InvalidHandle;
    }

    Framebuffer &Framebuffer::operator=(Framebuffer &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_framebuffer(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_name = std::move(other.m_name);
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<Framebuffer>, FramebufferFailure> Framebuffer::create(backend::RenderBackend &backend,
                                                                                       const std::string &name) {

        auto result = backend.create_framebuffer();
        if (!result) {
            return core::Fail(RIBBLE_ERROR(FramebufferFailure::CreationFailure, "Failed to create framebuffer"));
        }

        auto framebuffer = std::unique_ptr<Framebuffer>(new Framebuffer(backend, *result, name));
        RIBBLE_LOG_INFO("Created framebuffer '{}' (handle: {})", name.empty() ? "<unnamed>" : name, *result);
        return core::Result<std::unique_ptr<Framebuffer>, FramebufferFailure>(std::in_place, std::move(framebuffer));
    }

    void Framebuffer::bind() const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_framebuffer(m_handle);
        }
    }

    void Framebuffer::unbind() const { m_backend.bind_framebuffer(backend::InvalidHandle); }

    void Framebuffer::attach_texture(std::shared_ptr<Texture> texture, uint32_t attachment) {
        // This would need backend support for framebuffer attachments
        // For now, this is a placeholder
        // TODO: Add attach_texture_to_framebuffer to RenderBackend interface
    }

    void Framebuffer::resize(int width, int height) {
        // This would need backend support for framebuffer resizing
        // For now, this is a placeholder
        // TODO: Add resize_framebuffer to RenderBackend interface
    }

    bool Framebuffer::is_complete() const {
        // This would need backend support for framebuffer completeness checking
        // For now, this is a placeholder
        // TODO: Add check_framebuffer_completeness to RenderBackend interface
        return m_handle != backend::InvalidHandle;
    }

} // namespace ribble::render
