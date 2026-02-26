#include "opengl_state.h"
#include <cstring>
#include "opengl_conversions.h"

namespace backend {

    void OpenGLState::set_viewport(const Viewport &vp) {
        if (m_cache.viewport.x == vp.x && m_cache.viewport.y == vp.y && m_cache.viewport.width == vp.width &&
            m_cache.viewport.height == vp.height)
            return;
        m_cache.viewport = vp;
        glViewport(vp.x, vp.y, vp.width, vp.height);
    }

    void OpenGLState::set_scissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }

    void OpenGLState::set_scissor_enabled(bool enabled) {
        if (m_cache.scissorEnabled == enabled)
            return;
        m_cache.scissorEnabled = enabled;
        enabled ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
    }

    // ── Clear ─────────────────────────────────────────────────────────────────

    void OpenGLState::set_clear_color(const ClearColor &color) {
        if (m_cache.clearColor.r() == color.r() && m_cache.clearColor.g() == color.g() &&
            m_cache.clearColor.b() == color.b() && m_cache.clearColor.a() == color.a())
            return;
        m_cache.clearColor = color;
        glClearColor(color.r(), color.g(), color.b(), color.a());
    }

    void OpenGLState::set_clear_depth(float depth) {
        if (m_cache.clearDepth == depth)
            return;
        m_cache.clearDepth = depth;
        glClearDepthf(depth);
    }

    void OpenGLState::set_clear_stencil(int stencil) {
        if (m_cache.clearStencil == stencil)
            return;
        m_cache.clearStencil = stencil;
        glClearStencil(stencil);
    }

    void OpenGLState::clear(bool color, bool depth, bool stencil) {
        GLbitfield mask = 0;
        if (color)
            mask |= GL_COLOR_BUFFER_BIT;
        if (depth)
            mask |= GL_DEPTH_BUFFER_BIT;
        if (stencil)
            mask |= GL_STENCIL_BUFFER_BIT;
        if (mask)
            glClear(mask);
    }

    // ── Depth ─────────────────────────────────────────────────────────────────

    void OpenGLState::set_depth_test(bool enabled) {
        if (m_cache.depthTest == enabled)
            return;
        m_cache.depthTest = enabled;
        enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    }

    void OpenGLState::set_depth_write(bool enabled) {
        if (m_cache.depthWrite == enabled)
            return;
        m_cache.depthWrite = enabled;
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    }

    void OpenGLState::set_depth_func(DepthFunc func) {
        if (m_cache.depthFunc == func)
            return;
        m_cache.depthFunc = func;
        glDepthFunc(to_gl_depth_func(func));
    }

    // ── Blend ─────────────────────────────────────────────────────────────────

    void OpenGLState::set_blend(bool enabled) {
        if (m_cache.blend == enabled)
            return;
        m_cache.blend = enabled;
        enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
    }

    void OpenGLState::set_blend_func(BlendFactor src, BlendFactor dst) { set_blend_func_separate(src, dst, src, dst); }

    void OpenGLState::set_blend_func_separate(BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcAlpha,
                                              BlendFactor dstAlpha) {
        if (m_cache.blendSrcRgb == srcRgb && m_cache.blendDstRgb == dstRgb && m_cache.blendSrcAlpha == srcAlpha &&
            m_cache.blendDstAlpha == dstAlpha)
            return;
        m_cache.blendSrcRgb = srcRgb;
        m_cache.blendDstRgb = dstRgb;
        m_cache.blendSrcAlpha = srcAlpha;
        m_cache.blendDstAlpha = dstAlpha;
        glBlendFuncSeparate(to_gl_blend_factor(srcRgb), to_gl_blend_factor(dstRgb), to_gl_blend_factor(srcAlpha),
                            to_gl_blend_factor(dstAlpha));
    }

    void OpenGLState::set_blend_equation(BlendOp op) {
        if (m_cache.blendOp == op)
            return;
        m_cache.blendOp = op;
        glBlendEquation(to_gl_blend_op(op));
    }

    // ── Cull ──────────────────────────────────────────────────────────────────

    void OpenGLState::set_cull_face(CullMode mode) {
        if (m_cache.cullMode == mode)
            return;
        m_cache.cullMode = mode;
        if (mode == CullMode::None) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(mode == CullMode::Front ? GL_FRONT : GL_BACK);
        }
    }

    void OpenGLState::set_winding_order(WindingOrder order) {
        if (m_cache.windingOrder == order)
            return;
        m_cache.windingOrder = order;
        glFrontFace(order == WindingOrder::Clockwise ? GL_CW : GL_CCW);
    }

    void OpenGLState::set_program_point_size(bool enabled) {
        if (m_cache.programPointSize == enabled)
            return;
        m_cache.programPointSize = enabled;
        enabled ? glEnable(GL_PROGRAM_POINT_SIZE) : glDisable(GL_PROGRAM_POINT_SIZE);
    }

    // ── Bindings ──────────────────────────────────────────────────────────────

    void OpenGLState::bind_vao(GLuint vao) {
        if (m_cache.boundVao == vao)
            return;
        m_cache.boundVao = vao;
        glBindVertexArray(vao);
    }

    void OpenGLState::bind_vbo(GLuint vbo) {
        if (m_cache.boundVbo == vbo)
            return;
        m_cache.boundVbo = vbo;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void OpenGLState::bind_ebo(GLuint ebo) {
        if (m_cache.boundEbo == ebo)
            return;
        m_cache.boundEbo = ebo;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    void OpenGLState::bind_texture(GLenum target, GLuint texture, int unit) {
        if (m_cache.activeTextureUnit != unit) {
            m_cache.activeTextureUnit = unit;
            glActiveTexture(GL_TEXTURE0 + unit);
        }
        if (m_cache.boundTextures[unit] == texture && m_cache.boundTextureTargets[unit] == target)
            return;
        m_cache.boundTextures[unit] = texture;
        m_cache.boundTextureTargets[unit] = target;
        glBindTexture(target, texture);
    }

    void OpenGLState::bind_framebuffer(GLenum target, GLuint fbo) {
        if (target == GL_READ_FRAMEBUFFER) {
            if (m_cache.boundFboRead == fbo)
                return;
            m_cache.boundFboRead = fbo;
        } else if (target == GL_DRAW_FRAMEBUFFER) {
            if (m_cache.boundFboDraw == fbo)
                return;
            m_cache.boundFboDraw = fbo;
        } else { // GL_FRAMEBUFFER
            if (m_cache.boundFboRead == fbo && m_cache.boundFboDraw == fbo)
                return;
            m_cache.boundFboRead = m_cache.boundFboDraw = fbo;
        }
        glBindFramebuffer(target, fbo);
    }

    void OpenGLState::use_program(GLuint program) {
        if (m_cache.boundProgram == program)
            return;
        m_cache.boundProgram = program;
        glUseProgram(program);
    }

    void OpenGLState::reset() {
        m_cache = Cache{};
        m_dirty = false;
    }

} // namespace backend
