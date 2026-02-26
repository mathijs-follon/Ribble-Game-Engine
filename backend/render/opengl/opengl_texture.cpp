#include "opengl_texture.h"
#include <ribble/core/logger.h>
#include "opengl_conversions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace backend {

    OpenGLTexture::~OpenGLTexture() { destroy(); }

    OpenGLTexture::OpenGLTexture(OpenGLTexture &&o) noexcept : m_id(o.m_id), m_desc(o.m_desc) { o.m_id = 0; }

    OpenGLTexture &OpenGLTexture::operator=(OpenGLTexture &&o) noexcept {
        if (this != &o) {
            destroy();
            m_id = o.m_id;
            m_desc = o.m_desc;
            o.m_id = 0;
        }
        return *this;
    }

    ribble::core::Result<void, OpenGLTexture::Failure> OpenGLTexture::create(const TextureDesc &desc) {
        destroy();
        m_desc = desc;

        glGenTextures(1, &m_id);
        if (!m_id)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::CreationFailure, "glGenTextures failed"));

        const auto [internalFmt, fmt, type] = to_gl_texture_format(desc.format);

        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, desc.width, desc.height, 0, fmt, type, desc.data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_gl_texture_filter(desc.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_gl_texture_filter(desc.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_gl_texture_wrap(desc.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_gl_texture_wrap(desc.wrapT));

        if (desc.generateMipmaps && desc.data)
            glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        return ribble::core::Ok();
    }

    void OpenGLTexture::upload(const void *data, int level) {
        const auto [internalFmt, fmt, type] = to_gl_texture_format(m_desc.format);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_desc.width, m_desc.height, fmt, type, data);
        if (m_desc.generateMipmaps)
            glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ribble::core::Result<void, OpenGLTexture::Failure> OpenGLTexture::resize(int width, int height) {
        m_desc.width = width;
        m_desc.height = height;
        m_desc.data = nullptr;
        return create(m_desc);
    }

    void OpenGLTexture::bind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void OpenGLTexture::unbind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture::destroy() {
        if (m_id) {
            glDeleteTextures(1, &m_id);
            m_id = 0;
        }
    }

    ribble::core::Result<OpenGLTexture, OpenGLTexture::Failure>
    OpenGLTexture::load_from_file(const std::string &filepath, bool generateMipmaps) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(false); // GLTF UV (0,0) = top-left matches stb_image's default
        unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
        if (!data) {
            RIBBLE_LOG_ERROR("Failed to load image: {} - {}", filepath, stbi_failure_reason());
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::CreationFailure, "Failed to load image file: {}", filepath));
        }

        TextureFormat format = TextureFormat::RGBA8;
        switch (channels) {
            case 1:
                format = TextureFormat::R8;
                break;
            case 2:
                format = TextureFormat::RG8;
                break;
            case 3:
                format = TextureFormat::RGB8;
                break;
            case 4:
                format = TextureFormat::RGBA8;
                break;
            default:
                format = TextureFormat::RGBA8;
                break;
        }

        TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.data = data;
        desc.generateMipmaps = generateMipmaps;
        desc.minFilter = generateMipmaps ? TextureFilter::LinearMipmapLinear : TextureFilter::Linear;
        desc.magFilter = TextureFilter::Linear;
        desc.wrapS = TextureWrap::Repeat;
        desc.wrapT = TextureWrap::Repeat;

        OpenGLTexture texture;
        auto result = texture.create(desc);
        stbi_image_free(data);

        if (!result) {
            return ribble::core::Fail(result.error());
        }

        return ribble::core::Result<OpenGLTexture, OpenGLTexture::Failure>(std::in_place, std::move(texture));
    }

    void OpenGLTexture::set_wrap_mode(TextureWrap wrapS, TextureWrap wrapT) {
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_gl_texture_wrap(wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_gl_texture_wrap(wrapT));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture::set_filter_mode(TextureFilter minFilter, TextureFilter magFilter) {
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_gl_texture_filter(minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_gl_texture_filter(magFilter));
        m_desc.minFilter = minFilter;
        m_desc.magFilter = magFilter;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture::set_border_color(float r, float g, float b, float a) {
        glBindTexture(GL_TEXTURE_2D, m_id);
        float borderColor[] = {r, g, b, a};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture::generate_mipmaps() {
        glBindTexture(GL_TEXTURE_2D, m_id);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_desc.generateMipmaps = true;
    }

} // namespace backend
