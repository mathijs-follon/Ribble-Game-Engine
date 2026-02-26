#pragma once
#include <cstdint>

namespace backend {
    enum class PrimitiveTopology {
        Triangles,
        TriangleStrip,
        Lines,
        LineStrip,
        Points,
    };

    enum class WindingOrder {
        Clockwise,
        CounterClockwise,
    };

    enum class CullMode {
        None,
        Front,
        Back,
    };

    enum class DepthFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    enum class BlendFactor {
        Zero,
        One,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        SrcColor,
        OneMinusSrcColor,
    };

    enum class BlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class TextureFormat {
        R8,
        RG8,
        RGB8,
        RGBA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        Depth16,
        Depth24,
        Depth32F,
        Depth24Stencil8,
    };

    enum class TextureFilter {
        Nearest,
        Linear,
        NearestMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapNearest,
        LinearMipmapLinear,
    };

    enum class TextureWrap {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
    };

    enum class BufferUsage {
        Static,
        Dynamic,
        Stream,
    };

    enum class BufferType {
        Vertex,
        Index,
        Uniform,
        ShaderStorage,
    };

    enum class IndexType {
        UInt16,
        UInt32,
    };

    enum class ShaderStage {
        Vertex,
        Fragment,
        Geometry,
        Compute,
        TessellationControl,
        TessellationEvaluation,
    };

    enum class ShaderLanguage {
        GLSL, // OpenGL Shading Language
        HLSL, // High-Level Shading Language (DirectX)
        MSL, // Metal Shading Language
        SPIRV, // SPIR-V bytecode (Vulkan)
    };

    using RenderHandle = uint32_t;
    constexpr RenderHandle InvalidHandle = 0;

    /// Hint for window creation (which graphics API will be used)
    enum class GraphicsAPI {
        OpenGL,
        Vulkan,
        DirectX12,
        Metal,
    };
} // namespace backend
