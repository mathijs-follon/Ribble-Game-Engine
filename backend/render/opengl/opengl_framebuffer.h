#pragma once
#include <glad/gl.h>
#include <ribble/core/fail.h>
#include <vector>
#include "backend_types.h"
#include "opengl_texture.h"

namespace backend {

    struct FramebufferAttachment {
        TextureFormat format{TextureFormat::RGBA8};
    };

    struct FramebufferDesc {
        int width{1};
        int height{1};
        std::vector<FramebufferAttachment> colorAttachments; // One entry per color target
        bool depthStencil{true};
    };

    class OpenGLFramebuffer {
    public:
        enum class Failure {
            CreationFailure,
            IncompleteFramebuffer,
        };

        OpenGLFramebuffer() = default;
        ~OpenGLFramebuffer();

        OpenGLFramebuffer(const OpenGLFramebuffer &) = delete;
        OpenGLFramebuffer &operator=(const OpenGLFramebuffer &) = delete;
        OpenGLFramebuffer(OpenGLFramebuffer &&) noexcept;
        OpenGLFramebuffer &operator=(OpenGLFramebuffer &&) noexcept;

        ribble::core::Result<void, Failure> create(const FramebufferDesc &desc);
        ribble::core::Result<void, Failure> resize(int width, int height);
        void destroy();

        void bind() const;
        void unbind() const;

        [[nodiscard]] GLuint id() const { return m_id; }
        [[nodiscard]] int width() const { return m_desc.width; }
        [[nodiscard]] int height() const { return m_desc.height; }
        [[nodiscard]] bool is_valid() const { return m_id != 0; }
        [[nodiscard]] const OpenGLTexture &color_texture(size_t index = 0) const { return m_colorTextures[index]; }
        [[nodiscard]] const OpenGLTexture &depth_texture() const { return m_depthTexture; }

    private:
        GLuint m_id{0};
        FramebufferDesc m_desc{};
        std::vector<OpenGLTexture> m_colorTextures;
        OpenGLTexture m_depthTexture;
    };

} // namespace backend
