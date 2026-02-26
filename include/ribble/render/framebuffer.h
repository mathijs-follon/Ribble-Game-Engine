#pragma once
#include <memory>
#include <string>
#include "../../backend/common/backend_types.h"
#include "../../backend/common/render_backend.h"
#include "ribble/core/fail.h"

namespace ribble::render {
    class Texture; // Forward declaration

    enum class FramebufferFailure {
        CreationFailure,
        IncompleteFramebuffer,
    };

    /// High-level, backend-agnostic Framebuffer class
    class Framebuffer {
    public:
        static core::Result<std::unique_ptr<Framebuffer>, FramebufferFailure> create(backend::RenderBackend &backend,
                                                                                     const std::string &name = "");

        ~Framebuffer();

        Framebuffer(const Framebuffer &) = delete;
        Framebuffer &operator=(const Framebuffer &) = delete;
        Framebuffer(Framebuffer &&other) noexcept;
        Framebuffer &operator=(Framebuffer &&other) noexcept;

        /// Bind this framebuffer for rendering
        void bind() const;

        /// Unbind framebuffer (bind default)
        void unbind() const;

        /// Attach texture to framebuffer
        /// @param texture Texture to attach
        /// @param attachment Attachment point (color, depth, etc.)
        void attach_texture(std::shared_ptr<Texture> texture, uint32_t attachment);

        /// Resize framebuffer
        void resize(int width, int height);

        /// Check if framebuffer is complete
        [[nodiscard]] bool is_complete() const;

        /// Get backend framebuffer handle
        [[nodiscard]] backend::RenderHandle handle() const { return m_handle; }

        /// Check if framebuffer is valid
        [[nodiscard]] bool is_valid() const { return m_handle != backend::InvalidHandle; }

        /// Get framebuffer name
        [[nodiscard]] const std::string &name() const { return m_name; }

    private:
        Framebuffer(backend::RenderBackend &backend, backend::RenderHandle handle, const std::string &name);

        backend::RenderBackend &m_backend;
        backend::RenderHandle m_handle{backend::InvalidHandle};
        std::string m_name;
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(
        ribble::render::FramebufferFailure,
        case ribble::render::FramebufferFailure::CreationFailure : return "Framebuffer Creation Failure";
        case ribble::render::FramebufferFailure::IncompleteFramebuffer : return "Incomplete Framebuffer";);
