#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "../../backend/common/backend_types.h"
#include "../../backend/common/render_backend.h"
#include "ribble/core/fail.h"

namespace ribble::render {

    enum class TextureFailure {
        CreationFailure,
        LoadFailure,
        InvalidTexture,
    };

    /// High-level, backend-agnostic Texture class
    class Texture {
    public:
        /// Create texture from data
        static core::Result<std::unique_ptr<Texture>, TextureFailure> create(backend::RenderBackend &backend, int width,
                                                                             int height, backend::TextureFormat format,
                                                                             const void *data = nullptr,
                                                                             const std::string &name = "");

        /// Create texture from file
        static core::Result<std::unique_ptr<Texture>, TextureFailure> create_from_file(backend::RenderBackend &backend,
                                                                                       const std::string &filepath,
                                                                                       bool generateMipmaps = true,
                                                                                       const std::string &name = "");

        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        /// Bind texture to a texture unit
        void bind(int unit = 0) const;

        /// Unbind texture
        void unbind(int unit = 0) const;

        /// Set texture wrap mode
        void set_wrap_mode(backend::TextureWrap wrapS, backend::TextureWrap wrapT);

        /// Set texture filter mode
        void set_filter_mode(backend::TextureFilter minFilter, backend::TextureFilter magFilter);

        /// Set border color (for ClampToBorder wrap mode)
        void set_border_color(const glm::vec4 &color);

        /// Generate mipmaps
        void generate_mipmaps();

        /// Get backend texture handle
        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }

        /// Check if texture is valid
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

        /// Get texture name
        [[nodiscard]] const std::string &name() const { return m_name; }

        /// Get texture width
        [[nodiscard]] int width() const { return m_width; }

        /// Get texture height
        [[nodiscard]] int height() const { return m_height; }

    private:
        Texture(backend::RenderBackend &backend, backend::RenderHandle handle, int width, int height,
                const std::string &name);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        int m_width{0};
        int m_height{0};
        std::string m_name;
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(ribble::render::TextureFailure,
                      case ribble::render::TextureFailure::CreationFailure : return "Texture Creation Failure";
                      case ribble::render::TextureFailure::LoadFailure : return "Texture Load Failure";
                      case ribble::render::TextureFailure::InvalidTexture : return "Invalid Texture";);
