#include "opengl_texture.h"
#include "opengl_conversions.h"

namespace ribble::backend::opengl {

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

    core::Result<void, OpenGLTexture::Failure> OpenGLTexture::create(const TextureDesc &desc) {
        destroy();
        m_desc = desc;

        glGenTextures(1, &m_id);
        if (!m_id)
            return core::Fail(RIBBLE_ERROR(Failure::CreationFailure, "glGenTextures failed"));

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
        return core::Ok();
    }

    void OpenGLTexture::upload(const void *data, int level) {
        const auto [internalFmt, fmt, type] = to_gl_texture_format(m_desc.format);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_desc.width, m_desc.height, fmt, type, data);
        if (m_desc.generateMipmaps)
            glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    core::Result<void, OpenGLTexture::Failure> OpenGLTexture::resize(int width, int height) {
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

} // namespace ribble::backend::opengl
