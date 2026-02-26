#pragma once
#include <glad/gl.h>
#include <ribble/core/fail.h>
#include "backend_types.h"

namespace ribble::backend::opengl {

    struct TextureDesc {
        int width{1};
        int height{1};
        TextureFormat format{TextureFormat::RGBA8};
        TextureFilter minFilter{TextureFilter::LinearMipmapLinear};
        TextureFilter magFilter{TextureFilter::Linear};
        TextureWrap wrapS{TextureWrap::Repeat};
        TextureWrap wrapT{TextureWrap::Repeat};
        bool generateMipmaps{true};
        const void *data{nullptr}; // nullptr = allocate only
    };

    class OpenGLTexture {
    public:
        enum class Failure {
            CreationFailure,
            InvalidTexture,
        };

        OpenGLTexture() = default;
        ~OpenGLTexture();

        OpenGLTexture(const OpenGLTexture &) = delete;
        OpenGLTexture &operator=(const OpenGLTexture &) = delete;
        OpenGLTexture(OpenGLTexture &&) noexcept;
        OpenGLTexture &operator=(OpenGLTexture &&) noexcept;

        core::Result<void, Failure> create(const TextureDesc &desc);
        void destroy();

        void bind(int unit = 0) const;
        void unbind(int unit = 0) const;

        /// Replace pixel data (must match original format/size)
        void upload(const void *data, int level = 0);

        /// Resize — destroys and re-creates the texture
        core::Result<void, Failure> resize(int width, int height);

        [[nodiscard]] GLuint id() const { return m_id; }
        [[nodiscard]] int width() const { return m_desc.width; }
        [[nodiscard]] int height() const { return m_desc.height; }
        [[nodiscard]] TextureFormat format() const { return m_desc.format; }
        [[nodiscard]] bool is_valid() const { return m_id != 0; }

    private:
        GLuint m_id{0};
        TextureDesc m_desc{};
    };

} // namespace ribble::backend::opengl
