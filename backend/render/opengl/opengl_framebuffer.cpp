#include "opengl_framebuffer.h"
#include <ribble/core/logger.h>
#include "opengl_conversions.h"

namespace backend {

    OpenGLFramebuffer::~OpenGLFramebuffer() { destroy(); }

    OpenGLFramebuffer::OpenGLFramebuffer(OpenGLFramebuffer &&o) noexcept :
        m_id(o.m_id), m_desc(std::move(o.m_desc)), m_colorTextures(std::move(o.m_colorTextures)),
        m_depthTexture(std::move(o.m_depthTexture)) {
        o.m_id = 0;
    }

    OpenGLFramebuffer &OpenGLFramebuffer::operator=(OpenGLFramebuffer &&o) noexcept {
        if (this != &o) {
            destroy();
            m_id = o.m_id;
            m_desc = std::move(o.m_desc);
            m_colorTextures = std::move(o.m_colorTextures);
            m_depthTexture = std::move(o.m_depthTexture);
            o.m_id = 0;
        }
        return *this;
    }

    ribble::core::Result<void, OpenGLFramebuffer::Failure> OpenGLFramebuffer::create(const FramebufferDesc &desc) {
        destroy();
        m_desc = desc;

        glGenFramebuffers(1, &m_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);

        // Color attachments
        m_colorTextures.resize(desc.colorAttachments.size());
        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(desc.colorAttachments.size());

        for (size_t i = 0; i < desc.colorAttachments.size(); ++i) {
            TextureDesc td{};
            td.width = desc.width;
            td.height = desc.height;
            td.format = desc.colorAttachments[i].format;
            td.minFilter = TextureFilter::Linear;
            td.magFilter = TextureFilter::Linear;
            td.wrapS = TextureWrap::ClampToEdge;
            td.wrapT = TextureWrap::ClampToEdge;
            td.generateMipmaps = false;

            if (auto r = m_colorTextures[i].create(td); !r) {
                destroy();
                return ribble::core::Fail(
                        RIBBLE_ERROR(Failure::CreationFailure, "Failed to create color attachment {}", i));
            }

            const GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_colorTextures[i].id(), 0);
            drawBuffers.push_back(attachment);
        }

        if (!drawBuffers.empty())
            glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

        // Depth/stencil attachment
        if (desc.depthStencil) {
            TextureDesc dd{};
            dd.width = desc.width;
            dd.height = desc.height;
            dd.format = TextureFormat::Depth24Stencil8;
            dd.minFilter = TextureFilter::Nearest;
            dd.magFilter = TextureFilter::Nearest;
            dd.wrapS = TextureWrap::ClampToEdge;
            dd.wrapT = TextureWrap::ClampToEdge;
            dd.generateMipmaps = false;

            if (auto r = m_depthTexture.create(dd); !r) {
                destroy();
                return ribble::core::Fail(
                        RIBBLE_ERROR(Failure::CreationFailure, "Failed to create depth/stencil attachment"));
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture.id(), 0);
        }

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (status != GL_FRAMEBUFFER_COMPLETE) {
            destroy();
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::IncompleteFramebuffer, "Framebuffer incomplete, status: {:#x}", status));
        }

        return ribble::core::Ok();
    }

    ribble::core::Result<void, OpenGLFramebuffer::Failure> OpenGLFramebuffer::resize(int width, int height) {
        m_desc.width = width;
        m_desc.height = height;
        return create(m_desc);
    }

    void OpenGLFramebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
    void OpenGLFramebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void OpenGLFramebuffer::destroy() {
        m_colorTextures.clear();
        m_depthTexture.destroy();
        if (m_id) {
            glDeleteFramebuffers(1, &m_id);
            m_id = 0;
        }
    }

} // namespace backend
