#pragma once

#include <vulkan/vulkan.h>
#include "../../common/backend_types.h"
#include "ribble/render/color.h"

namespace backend {

    constexpr VkFormat to_vk_format(TextureFormat format) {
        switch (format) {
            case TextureFormat::R8: return VK_FORMAT_R8_UNORM;
            case TextureFormat::RG8: return VK_FORMAT_R8G8_UNORM;
            case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_UNORM;
            case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::R16F: return VK_FORMAT_R16_SFLOAT;
            case TextureFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
            case TextureFormat::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
            case TextureFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
            case TextureFormat::R32F: return VK_FORMAT_R32_SFLOAT;
            case TextureFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
            case TextureFormat::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
            case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case TextureFormat::Depth16: return VK_FORMAT_D16_UNORM;
            case TextureFormat::Depth24: return VK_FORMAT_D32_SFLOAT;
            case TextureFormat::Depth32F: return VK_FORMAT_D32_SFLOAT;
            case TextureFormat::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
            default: return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    constexpr VkPrimitiveTopology to_vk_primitive(PrimitiveTopology topo) {
        switch (topo) {
            case PrimitiveTopology::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case PrimitiveTopology::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopology::Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    constexpr VkIndexType to_vk_index_type(IndexType type) {
        switch (type) {
            case IndexType::UInt16: return VK_INDEX_TYPE_UINT16;
            case IndexType::UInt32: return VK_INDEX_TYPE_UINT32;
            default: return VK_INDEX_TYPE_UINT32;
        }
    }

    inline void color_to_vk_clear_color(const ribble::render::ColorRGBA &c, VkClearColorValue &out) {
        out.float32[0] = c.r();
        out.float32[1] = c.g();
        out.float32[2] = c.b();
        out.float32[3] = c.a();
    }

    constexpr VkShaderStageFlagBits to_vk_shader_stage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderStage::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderStage::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            default: return VK_SHADER_STAGE_VERTEX_BIT;
        }
    }

    constexpr VkCompareOp to_vk_compare_op(DepthFunc func) {
        switch (func) {
            case DepthFunc::Never: return VK_COMPARE_OP_NEVER;
            case DepthFunc::Less: return VK_COMPARE_OP_LESS;
            case DepthFunc::Equal: return VK_COMPARE_OP_EQUAL;
            case DepthFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
            case DepthFunc::Greater: return VK_COMPARE_OP_GREATER;
            case DepthFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case DepthFunc::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case DepthFunc::Always: return VK_COMPARE_OP_ALWAYS;
            default: return VK_COMPARE_OP_LESS;
        }
    }

    constexpr VkBlendFactor to_vk_blend_factor(BlendFactor f) {
        switch (f) {
            case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
            case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
            case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            default: return VK_BLEND_FACTOR_ONE;
        }
    }

    constexpr VkBlendOp to_vk_blend_op(BlendOp op) {
        switch (op) {
            case BlendOp::Add: return VK_BLEND_OP_ADD;
            case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
            case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
            case BlendOp::Min: return VK_BLEND_OP_MIN;
            case BlendOp::Max: return VK_BLEND_OP_MAX;
            default: return VK_BLEND_OP_ADD;
        }
    }

    constexpr VkCullModeFlags to_vk_cull_mode(CullMode mode) {
        switch (mode) {
            case CullMode::None: return VK_CULL_MODE_NONE;
            case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
            default: return VK_CULL_MODE_NONE;
        }
    }

    constexpr VkFrontFace to_vk_front_face(WindingOrder order) {
        return order == WindingOrder::CounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    }

} // namespace backend
