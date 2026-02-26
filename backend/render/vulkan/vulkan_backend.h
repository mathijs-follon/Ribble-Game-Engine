#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "../../common/render_backend.h"
#include "ribble/render/color.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync.h"
#include "vulkan_texture.h"

namespace backend {

    struct VulkanVertexArray {
        RenderHandle vertexBuffer{InvalidHandle};
        RenderHandle indexBuffer{InvalidHandle};
    };

    class VulkanBackend : public RenderBackend {
    public:
        explicit VulkanBackend(std::unique_ptr<VulkanContext> context);
        ~VulkanBackend() override;

        VulkanBackend(const VulkanBackend &) = delete;
        VulkanBackend &operator=(const VulkanBackend &) = delete;

        ribble::core::Result<void, Failure> initialize(ribble::window::WindowContext &windowContext, int width,
                                                       int height) override;
        ribble::core::Result<void, Failure> shutdown() override;

        ribble::core::Result<void, Failure> begin_frame() override;
        ribble::core::Result<void, Failure> end_frame() override;

        ribble::core::Result<void, Failure> set_viewport(const Viewport &viewport) override;
        ribble::core::Result<void, Failure> set_clear_color(const ribble::render::ColorRGBA &color) override;
        ribble::core::Result<void, Failure> clear() override;

        ribble::core::Result<void, Failure> on_resize(int width, int height) override;

        [[nodiscard]] const char *backend_name() const override;

        // Resource creation - return errors (not yet implemented for Vulkan)
        ribble::core::Result<RenderHandle, Failure> create_shader(const ShaderSource &source) override;
        void destroy_shader(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_texture(int width, int height, TextureFormat format,
                                                                   const void *data = nullptr) override;
        void destroy_texture(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_buffer(BufferType type, BufferUsage usage, size_t size,
                                                                  const void *data = nullptr) override;
        void destroy_buffer(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_vertex_array() override;
        void destroy_vertex_array(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_framebuffer() override;
        void destroy_framebuffer(RenderHandle handle) override;

        ribble::core::Result<RenderHandle, Failure> create_pipeline(RenderHandle shaderHandle) override;
        void destroy_pipeline(RenderHandle handle) override;

        // Resource binding
        ribble::core::Result<void, Failure> bind_pipeline(RenderHandle pipelineHandle) override;
        ribble::core::Result<void, Failure> bind_texture(RenderHandle textureHandle, int unit = 0) override;
        ribble::core::Result<void, Failure> bind_buffer(RenderHandle bufferHandle, BufferType type) override;
        ribble::core::Result<void, Failure> bind_vertex_array(RenderHandle vertexArrayHandle) override;
        ribble::core::Result<void, Failure> bind_framebuffer(RenderHandle framebufferHandle) override;

        // Draw methods
        ribble::core::Result<void, Failure>
        draw_indexed(RenderHandle vertexArrayHandle, uint32_t indexCount, IndexType indexType, uint32_t indexOffset = 0,
                     int32_t baseVertex = 0, PrimitiveTopology topology = PrimitiveTopology::Triangles) override;
        ribble::core::Result<void, Failure>
        draw_arrays(RenderHandle vertexArrayHandle, uint32_t vertexCount, uint32_t vertexOffset = 0,
                    PrimitiveTopology topology = PrimitiveTopology::Triangles) override;
        ribble::core::Result<void, Failure>
        draw_instanced(RenderHandle vertexArrayHandle, uint32_t indexCount, uint32_t instanceCount,
                       IndexType indexType = IndexType::UInt32, uint32_t indexOffset = 0, int32_t baseVertex = 0,
                       PrimitiveTopology topology = PrimitiveTopology::Triangles) override;

        // Render state
        ribble::core::Result<void, Failure> set_depth_test(bool enabled) override;
        ribble::core::Result<void, Failure> set_depth_write(bool enabled) override;
        ribble::core::Result<void, Failure> set_depth_func(DepthFunc func) override;
        ribble::core::Result<void, Failure> set_blend(bool enabled) override;
        ribble::core::Result<void, Failure> set_blend_func(BlendFactor src, BlendFactor dst) override;
        ribble::core::Result<void, Failure> set_blend_op(BlendOp op) override;
        ribble::core::Result<void, Failure> set_cull_mode(CullMode mode) override;
        ribble::core::Result<void, Failure> set_winding_order(WindingOrder order) override;
        ribble::core::Result<void, Failure> set_program_point_size(bool enabled) override;

        // Uniforms
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        int value) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        float value) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y, float z) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name, float x,
                                                        float y, float z, float w) override;
        ribble::core::Result<void, Failure> set_uniform(RenderHandle shaderHandle, const std::string &name,
                                                        const float *matrixData, bool transpose = false) override;

    private:
        bool create_instance();
        bool pick_physical_device();
        bool create_logical_device();
        bool create_render_pass();
        bool create_framebuffers();
        bool create_command_pool();
        bool create_command_buffers();
        void begin_render_pass();
        void end_render_pass();
        VkCommandBuffer current_command_buffer();
        bool create_default_pipeline();
        void cleanup_swapchain();
        void recreate_swapchain();

        std::unique_ptr<VulkanContext> m_context;

        VkInstance m_instance{VK_NULL_HANDLE};
        VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        VkSurfaceKHR m_surface{VK_NULL_HANDLE};
        VkQueue m_graphicsQueue{VK_NULL_HANDLE};
        VkQueue m_presentQueue{VK_NULL_HANDLE};

        VulkanSwapchain m_swapchain;
        VkRenderPass m_renderPass{VK_NULL_HANDLE};
        std::vector<VkFramebuffer> m_swapchainFramebuffers;
        VkCommandPool m_commandPool{VK_NULL_HANDLE};
        std::vector<VkCommandBuffer> m_commandBuffers;

        VulkanSync m_sync;

        ribble::window::WindowContext *m_windowContext{nullptr};
        ribble::render::ColorRGBA m_clearColor{0.1f, 0.1f, 0.1f, 1.f};
        Viewport m_viewport{};
        int m_width{0};
        int m_height{0};

        static constexpr size_t MaxFramesInFlight = 2;
        size_t m_currentFrame{0};
        uint32_t m_currentImageIndex{0};
        bool m_frameAcquired{false};
        bool m_inRenderPass{false};

        // Resource storage
        RenderHandle m_nextShaderHandle{1};
        RenderHandle m_nextTextureHandle{1};
        RenderHandle m_nextBufferHandle{1};
        RenderHandle m_nextVertexArrayHandle{1};
        RenderHandle m_nextFramebufferHandle{1};
        RenderHandle m_nextPipelineHandle{1};
        std::unordered_map<RenderHandle, std::unique_ptr<VulkanShader>> m_shaders;
        std::unordered_map<RenderHandle, std::unique_ptr<VulkanTexture>> m_textures;
        std::unordered_map<RenderHandle, std::unique_ptr<VulkanBuffer>> m_buffers;
        std::unordered_map<RenderHandle, VulkanVertexArray> m_vertexArrays;
        std::unordered_map<RenderHandle, std::unique_ptr<VulkanPipeline>> m_pipelines;
        std::unordered_map<RenderHandle, RenderHandle> m_pipelineToShader;

        // Current bind state
        RenderHandle m_boundPipeline{InvalidHandle};
        RenderHandle m_boundVertexArray{InvalidHandle};
        RenderHandle m_boundFramebuffer{InvalidHandle};

        // Render state (for pipeline creation / dynamic state)
        bool m_depthTest{true};
        bool m_depthWrite{true};
        DepthFunc m_depthFunc{DepthFunc::Less};
        bool m_blend{false};
        BlendFactor m_blendSrc{BlendFactor::SrcAlpha};
        BlendFactor m_blendDst{BlendFactor::OneMinusSrcAlpha};
        CullMode m_cullMode{CullMode::Back};
        WindingOrder m_windingOrder{WindingOrder::CounterClockwise};

        // Push constant data for uniforms
        alignas(64) float m_pushConstantMVP[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        float m_pushConstantColor[4]{1, 1, 1, 1};
    };

} // namespace backend
