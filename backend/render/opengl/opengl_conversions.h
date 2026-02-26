#pragma once
#include <glad/gl.h>
#include "backend_types.h"

namespace backend {

    [[nodiscard]] inline GLenum to_gl_primitive(PrimitiveTopology topology) {
        switch (topology) {
            case PrimitiveTopology::Triangles:
                return GL_TRIANGLES;
            case PrimitiveTopology::TriangleStrip:
                return GL_TRIANGLE_STRIP;
            case PrimitiveTopology::Lines:
                return GL_LINES;
            case PrimitiveTopology::LineStrip:
                return GL_LINE_STRIP;
            case PrimitiveTopology::Points:
                return GL_POINTS;
            default:
                return GL_TRIANGLES;
        }
    }

    [[nodiscard]] inline GLenum to_gl_depth_func(DepthFunc func) {
        switch (func) {
            case DepthFunc::Never:
                return GL_NEVER;
            case DepthFunc::Less:
                return GL_LESS;
            case DepthFunc::Equal:
                return GL_EQUAL;
            case DepthFunc::LessEqual:
                return GL_LEQUAL;
            case DepthFunc::Greater:
                return GL_GREATER;
            case DepthFunc::NotEqual:
                return GL_NOTEQUAL;
            case DepthFunc::GreaterEqual:
                return GL_GEQUAL;
            case DepthFunc::Always:
                return GL_ALWAYS;
            default:
                return GL_LESS;
        }
    }

    [[nodiscard]] inline GLenum to_gl_blend_factor(BlendFactor factor) {
        switch (factor) {
            case BlendFactor::Zero:
                return GL_ZERO;
            case BlendFactor::One:
                return GL_ONE;
            case BlendFactor::SrcAlpha:
                return GL_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return GL_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return GL_ONE_MINUS_DST_ALPHA;
            case BlendFactor::SrcColor:
                return GL_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return GL_ONE_MINUS_SRC_COLOR;
            default:
                return GL_ONE;
        }
    }

    [[nodiscard]] inline GLenum to_gl_blend_op(BlendOp op) {
        switch (op) {
            case BlendOp::Add:
                return GL_FUNC_ADD;
            case BlendOp::Subtract:
                return GL_FUNC_SUBTRACT;
            case BlendOp::ReverseSubtract:
                return GL_FUNC_REVERSE_SUBTRACT;
            case BlendOp::Min:
                return GL_MIN;
            case BlendOp::Max:
                return GL_MAX;
            default:
                return GL_FUNC_ADD;
        }
    }

    [[nodiscard]] inline GLenum to_gl_texture_filter(TextureFilter filter) {
        switch (filter) {
            case TextureFilter::Nearest:
                return GL_NEAREST;
            case TextureFilter::Linear:
                return GL_LINEAR;
            case TextureFilter::NearestMipmapNearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case TextureFilter::NearestMipmapLinear:
                return GL_NEAREST_MIPMAP_LINEAR;
            case TextureFilter::LinearMipmapNearest:
                return GL_LINEAR_MIPMAP_NEAREST;
            case TextureFilter::LinearMipmapLinear:
                return GL_LINEAR_MIPMAP_LINEAR;
            default:
                return GL_LINEAR;
        }
    }

    [[nodiscard]] inline GLenum to_gl_texture_wrap(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat:
                return GL_REPEAT;
            case TextureWrap::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case TextureWrap::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case TextureWrap::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
            default:
                return GL_REPEAT;
        }
    }

    [[nodiscard]] inline GLenum to_gl_buffer_usage(BufferUsage usage) {
        switch (usage) {
            case BufferUsage::Static:
                return GL_STATIC_DRAW;
            case BufferUsage::Dynamic:
                return GL_DYNAMIC_DRAW;
            case BufferUsage::Stream:
                return GL_STREAM_DRAW;
            default:
                return GL_STATIC_DRAW;
        }
    }

    [[nodiscard]] inline GLenum to_gl_buffer_type(BufferType type) {
        switch (type) {
            case BufferType::Vertex:
                return GL_ARRAY_BUFFER;
            case BufferType::Index:
                return GL_ELEMENT_ARRAY_BUFFER;
            case BufferType::Uniform:
                return GL_UNIFORM_BUFFER;
            case BufferType::ShaderStorage:
                return GL_SHADER_STORAGE_BUFFER;
            default:
                return GL_ARRAY_BUFFER;
        }
    }

    [[nodiscard]] inline GLenum to_gl_index_type(IndexType type) {
        switch (type) {
            case IndexType::UInt16:
                return GL_UNSIGNED_SHORT;
            case IndexType::UInt32:
                return GL_UNSIGNED_INT;
            default:
                return GL_UNSIGNED_INT;
        }
    }

    [[nodiscard]] inline GLenum to_gl_shader_stage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return GL_VERTEX_SHADER;
            case ShaderStage::Fragment:
                return GL_FRAGMENT_SHADER;
            case ShaderStage::Geometry:
                return GL_GEOMETRY_SHADER;
            case ShaderStage::Compute:
                return GL_COMPUTE_SHADER;
            case ShaderStage::TessellationControl:
                return GL_TESS_CONTROL_SHADER;
            case ShaderStage::TessellationEvaluation:
                return GL_TESS_EVALUATION_SHADER;
            default:
                return GL_VERTEX_SHADER;
        }
    }

    struct GLTextureFormat {
        GLenum internalFormat;
        GLenum format;
        GLenum type;
    };

    [[nodiscard]] inline GLTextureFormat to_gl_texture_format(TextureFormat fmt) {
        switch (fmt) {
            case TextureFormat::R8:
                return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};
            case TextureFormat::RG8:
                return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE};
            case TextureFormat::RGB8:
                return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE};
            case TextureFormat::RGBA8:
                return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            case TextureFormat::R16F:
                return {GL_R16F, GL_RED, GL_HALF_FLOAT};
            case TextureFormat::RG16F:
                return {GL_RG16F, GL_RG, GL_HALF_FLOAT};
            case TextureFormat::RGB16F:
                return {GL_RGB16F, GL_RGB, GL_HALF_FLOAT};
            case TextureFormat::RGBA16F:
                return {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT};
            case TextureFormat::R32F:
                return {GL_R32F, GL_RED, GL_FLOAT};
            case TextureFormat::RG32F:
                return {GL_RG32F, GL_RG, GL_FLOAT};
            case TextureFormat::RGB32F:
                return {GL_RGB32F, GL_RGB, GL_FLOAT};
            case TextureFormat::RGBA32F:
                return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
            case TextureFormat::Depth16:
                return {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT};
            case TextureFormat::Depth24:
                return {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT};
            case TextureFormat::Depth32F:
                return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT};
            case TextureFormat::Depth24Stencil8:
                return {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8};
            default:
                return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
        }
    }

} // namespace backend
