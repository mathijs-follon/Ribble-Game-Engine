#include "ribble/render/texture.h"
#include "../../backend/render/opengl/opengl_texture.h"
#include "ribble/core/logger.h"

namespace ribble::render {

    Texture::Texture(backend::RenderBackend &backend, backend::RenderHandle handle, int width, int height,
                     const std::string &name) :
        m_backend(backend), m_handle(handle), m_width(width), m_height(height), m_name(name) {}

    Texture::~Texture() {
        if (m_handle != backend::InvalidHandle) {
            m_backend.destroy_texture(m_handle);
        }
    }

    Texture::Texture(Texture &&other) noexcept :
        m_backend(other.m_backend), m_handle(other.m_handle), m_width(other.m_width), m_height(other.m_height),
        m_name(std::move(other.m_name)) {
        other.m_handle = backend::InvalidHandle;
    }

    Texture &Texture::operator=(Texture &&other) noexcept {
        if (this != &other) {
            if (m_handle != backend::InvalidHandle) {
                m_backend.destroy_texture(m_handle);
            }
            // Note: m_backend is a reference, cannot be reassigned
            m_handle = other.m_handle;
            m_width = other.m_width;
            m_height = other.m_height;
            m_name = std::move(other.m_name);
            other.m_handle = backend::InvalidHandle;
        }
        return *this;
    }

    core::Result<std::unique_ptr<Texture>, TextureFailure> Texture::create(backend::RenderBackend &backend, int width,
                                                                           int height, backend::TextureFormat format,
                                                                           const void *data, const std::string &name) {

        auto result = backend.create_texture(width, height, format, data);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(TextureFailure::CreationFailure, "Failed to create texture: {}",
                                           result.error().message));
        }

        auto texture = std::unique_ptr<Texture>(new Texture(backend, *result, width, height, name));
        RIBBLE_LOG_INFO("Created texture '{}' (handle: {}, {}x{})", name.empty() ? "<unnamed>" : name, *result, width,
                        height);
        return core::Result<std::unique_ptr<Texture>, TextureFailure>(std::in_place, std::move(texture));
    }

    core::Result<std::unique_ptr<Texture>, TextureFailure> Texture::create_from_file(backend::RenderBackend &backend,
                                                                                     const std::string &filepath,
                                                                                     bool generateMipmaps,
                                                                                     const std::string &name) {

        // For now, we'll use OpenGLTexture's load_from_file directly
        // In a full implementation, this would go through the backend
        // For OpenGL backend, we can cast and use the OpenGL-specific method
        auto textureResult = backend::OpenGLTexture::load_from_file(filepath, generateMipmaps);
        if (!textureResult) {
            return core::Fail(
                    RIBBLE_ERROR(TextureFailure::LoadFailure, "Failed to load texture from file: {}", filepath));
        }

        // Create a texture handle through the backend
        // For now, this is a simplified approach - in a full implementation,
        // the backend would handle file loading
        auto texture = std::move(*textureResult);
        int width = texture.width();
        int height = texture.height();

        // Create through backend (this will create a new texture, which is not ideal)
        // TODO: Better integration with backend resource management
        auto handleResult = backend.create_texture(width, height, texture.format(), nullptr);
        if (!handleResult) {
            return core::Fail(RIBBLE_ERROR(TextureFailure::CreationFailure, "Failed to create texture handle"));
        }

        auto result = std::unique_ptr<Texture>(
                new Texture(backend, *handleResult, width, height, name.empty() ? filepath : name));
        RIBBLE_LOG_INFO("Loaded texture from file '{}' (handle: {}, {}x{})", filepath, *handleResult, width, height);
        return core::Result<std::unique_ptr<Texture>, TextureFailure>(std::in_place, std::move(result));
    }

    void Texture::bind(int unit) const {
        if (m_handle != backend::InvalidHandle) {
            m_backend.bind_texture(m_handle, unit);
        }
    }

    void Texture::unbind(int unit) const { m_backend.bind_texture(backend::InvalidHandle, unit); }

    void Texture::set_wrap_mode(backend::TextureWrap wrapS, backend::TextureWrap wrapT) {
        // This would need backend support for setting texture parameters
        // For now, this is a placeholder
        // TODO: Add set_texture_wrap_mode to RenderBackend interface
    }

    void Texture::set_filter_mode(backend::TextureFilter minFilter, backend::TextureFilter magFilter) {
        // This would need backend support for setting texture parameters
        // For now, this is a placeholder
        // TODO: Add set_texture_filter_mode to RenderBackend interface
    }

    void Texture::set_border_color(const glm::vec4 &color) {
        // This would need backend support for setting texture parameters
        // For now, this is a placeholder
        // TODO: Add set_texture_border_color to RenderBackend interface
    }

    void Texture::generate_mipmaps() {
        // This would need backend support for generating mipmaps
        // For now, this is a placeholder
        // TODO: Add generate_texture_mipmaps to RenderBackend interface
    }

} // namespace ribble::render
