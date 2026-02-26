#pragma once
#include <glad/gl.h>
#include "backend_types.h"
#include "render_backend.h"
#include "ribble/render/color.h"

namespace ribble::backend::opengl {

    using ClearColor = render::ColorRGBA;

    /// Caches OpenGL state to avoid redundant driver calls.
    /// All RenderBackend state changes should go through this.
    class OpenGLState {
    public:
        OpenGLState() = default;
        ~OpenGLState() = default;

        OpenGLState(const OpenGLState &) = delete;
        OpenGLState &operator=(const OpenGLState &) = delete;

        // ── Viewport & scissor ────────────────────────────────────────────────
        void set_viewport(const Viewport &vp);
        void set_scissor(int x, int y, int width, int height);
        void set_scissor_enabled(bool enabled);

        // ── Clear ─────────────────────────────────────────────────────────────
        void set_clear_color(const ClearColor &color);
        void set_clear_depth(float depth);
        void set_clear_stencil(int stencil);
        void clear(bool color, bool depth, bool stencil);

        // ── Depth ─────────────────────────────────────────────────────────────
        void set_depth_test(bool enabled);
        void set_depth_write(bool enabled);
        void set_depth_func(DepthFunc func);

        // ── Blending ──────────────────────────────────────────────────────────
        void set_blend(bool enabled);
        void set_blend_func(BlendFactor src, BlendFactor dst);
        void set_blend_func_separate(BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcAlpha,
                                     BlendFactor dstAlpha);
        void set_blend_equation(BlendOp op);

        // ── Culling ───────────────────────────────────────────────────────────
        void set_cull_face(CullMode mode);
        void set_winding_order(WindingOrder order);

        // ── Binding helpers (with cache) ──────────────────────────────────────
        void bind_vao(GLuint vao);
        void bind_vbo(GLuint vbo);
        void bind_ebo(GLuint ebo);
        void bind_texture(GLenum target, GLuint texture, int unit = 0);
        void bind_framebuffer(GLenum target, GLuint fbo);
        void use_program(GLuint program);

        void reset(); // Force full re-apply (e.g. after context loss)

    private:
        struct Cache {
            // Viewport
            Viewport viewport{};
            bool scissorEnabled{false};

            // Clear
            ClearColor clearColor{0.f, 0.f, 0.f, 1.f}; // Uses ColorRGBA which stores as glm::vec4
            float clearDepth{1.f};
            int clearStencil{0};

            // Depth
            bool depthTest{false};
            bool depthWrite{true};
            DepthFunc depthFunc{DepthFunc::Less};

            // Blend
            bool blend{false};
            BlendFactor blendSrcRgb{BlendFactor::One};
            BlendFactor blendDstRgb{BlendFactor::Zero};
            BlendFactor blendSrcAlpha{BlendFactor::One};
            BlendFactor blendDstAlpha{BlendFactor::Zero};
            BlendOp blendOp{BlendOp::Add};

            // Cull
            CullMode cullMode{CullMode::None};
            WindingOrder windingOrder{WindingOrder::CounterClockwise};

            // Bindings
            GLuint boundVao{0};
            GLuint boundVbo{0};
            GLuint boundEbo{0};
            GLuint boundFboRead{0};
            GLuint boundFboDraw{0};
            GLuint boundProgram{0};
            GLuint boundTextures[32]{};
            GLenum boundTextureTargets[32]{};
            int activeTextureUnit{0};
        } m_cache;

        bool m_dirty{true}; // True until first reset() / apply
    };

} // namespace ribble::backend::opengl
