#include "vulkan_backend.h"
#include "vulkan_conversions.h"
#include "vulkan_default_shaders.h"
#include "../../common/backend_types.h"
#include "../../common/window_events.h"
#include <ribble/core/logger.h>
#include <cstring>
#include <set>

namespace backend {

    namespace {
        const std::vector<const char *> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        bool check_validation_support() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> layers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
            // Validation layers optional for now
            return false;
        }

        bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            bool hasGraphics = false;
            bool hasPresent = false;
            for (uint32_t i = 0; i < queueFamilyCount; ++i) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    hasGraphics = true;

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport)
                    hasPresent = true;
            }

            if (!hasGraphics || !hasPresent)
                return false;

            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

            std::set<std::string> required(kDeviceExtensions.begin(), kDeviceExtensions.end());
            for (const auto &ext : extensions) {
                required.erase(ext.extensionName);
            }
            return required.empty();
        }

        struct QueueFamilyIndices {
            uint32_t graphics{UINT32_MAX};
            uint32_t present{UINT32_MAX};
        };

        QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
            QueueFamilyIndices indices;
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            for (uint32_t i = 0; i < queueFamilyCount; ++i) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphics = i;

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport)
                    indices.present = i;

                if (indices.graphics != UINT32_MAX && indices.present != UINT32_MAX)
                    break;
            }
            return indices;
        }
    } // namespace

    VulkanBackend::VulkanBackend(std::unique_ptr<VulkanContext> context) : m_context(std::move(context)) {}

    VulkanBackend::~VulkanBackend() {
        if (m_initialized)
            VulkanBackend::shutdown();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::initialize(ribble::window::WindowContext &windowContext, int width, int height) {
        RenderBackend::initialize(windowContext, width, height);
        m_windowContext = &windowContext;
        m_width = width;
        m_height = height;
        m_viewport = {0, 0, width, height};

        if (!create_instance())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan instance"));

        auto surfaceResult = m_context->create_surface(m_instance, windowContext);
        if (!surfaceResult)
            return ribble::core::Fail(surfaceResult.error());
        m_surface = surfaceResult.value();

        if (!pick_physical_device())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to find suitable Vulkan device"));

        if (!create_logical_device())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan logical device"));

        auto queueIndices = find_queue_families(m_physicalDevice, m_surface);

        VulkanSwapchain::CreateInfo swapInfo{};
        swapInfo.physicalDevice = m_physicalDevice;
        swapInfo.device = m_device;
        swapInfo.surface = m_surface;
        swapInfo.graphicsQueueFamily = queueIndices.graphics;
        swapInfo.presentQueueFamily = queueIndices.present;
        swapInfo.width = width;
        swapInfo.height = height;

        if (!m_swapchain.create(swapInfo))
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan swapchain"));

        if (!create_render_pass())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan render pass"));

        if (!create_framebuffers())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan framebuffers"));

        if (!create_command_pool())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan command pool"));

        if (!create_command_buffers())
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan command buffers"));

        if (!m_sync.create(m_device, MaxFramesInFlight))
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::InitializationFailure, "Failed to create Vulkan sync objects"));

        if (!create_default_pipeline()) {
            RIBBLE_LOG_WARNING("Vulkan default pipeline not created (glslang may be missing)");
        }

        windowContext.event_bus()->subscribe<WindowResizeEvent>(
                [this](const std::shared_ptr<ribble::core::Event> &baseEvt) {
                    const auto &evt = static_cast<const WindowResizeEvent &>(*baseEvt);
                    if (m_windowContext && evt.width() > 0 && evt.height() > 0)
                        on_resize(evt.width(), evt.height());
                });

        m_initialized = true;
        RIBBLE_LOG_INFO("Vulkan backend initialized: {}", m_context->backend_name());
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::shutdown() {
        if (!m_initialized)
            return ribble::core::Ok();

        vkDeviceWaitIdle(m_device);

        m_pipelines.clear();
        m_buffers.clear();
        m_textures.clear();
        m_shaders.clear();
        m_vertexArrays.clear();

        m_sync.destroy(m_device);
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        cleanup_swapchain();
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        m_device = VK_NULL_HANDLE;
        m_instance = VK_NULL_HANDLE;
        m_surface = VK_NULL_HANDLE;
        m_physicalDevice = VK_NULL_HANDLE;
        m_windowContext = nullptr;
        m_initialized = false;

        RIBBLE_LOG_INFO("Vulkan backend shut down.");
        return ribble::core::Ok();
    }

    bool VulkanBackend::create_instance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Ribble";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Ribble";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        std::vector<const char *> extensions;
        m_context->get_required_extensions(extensions);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;

        return vkCreateInstance(&createInfo, nullptr, &m_instance) == VK_SUCCESS;
    }

    bool VulkanBackend::pick_physical_device() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            return false;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto &device : devices) {
            if (is_device_suitable(device, m_surface)) {
                m_physicalDevice = device;
                return true;
            }
        }
        return false;
    }

    bool VulkanBackend::create_logical_device() {
        auto indices = find_queue_families(m_physicalDevice, m_surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueFamilies = {indices.graphics, indices.present};

        float queuePriority = 1.0f;
        for (uint32_t family : uniqueFamilies) {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = family;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
            return false;

        vkGetDeviceQueue(m_device, indices.graphics, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.present, 0, &m_presentQueue);
        return true;
    }

    bool VulkanBackend::create_render_pass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapchain.imageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        return vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) == VK_SUCCESS;
    }

    bool VulkanBackend::create_framebuffers() {
        m_swapchainFramebuffers.resize(m_swapchain.imageCount());
        for (size_t i = 0; i < m_swapchain.imageCount(); ++i) {
            VkImageView attachments[] = {m_swapchain.imageViews()[i]};

            VkFramebufferCreateInfo fbInfo{};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = m_renderPass;
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments = attachments;
            fbInfo.width = m_swapchain.extent().width;
            fbInfo.height = m_swapchain.extent().height;
            fbInfo.layers = 1;

            if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
                for (size_t j = 0; j < i; ++j)
                    vkDestroyFramebuffer(m_device, m_swapchainFramebuffers[j], nullptr);
                m_swapchainFramebuffers.clear();
                return false;
            }
        }
        return true;
    }

    bool VulkanBackend::create_command_pool() {
        auto indices = find_queue_families(m_physicalDevice, m_surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = indices.graphics;

        return vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS;
    }

    bool VulkanBackend::create_command_buffers() {
        m_commandBuffers.resize(MaxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        return vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) == VK_SUCCESS;
    }

    void VulkanBackend::begin_render_pass() {
        if (m_inRenderPass || !m_frameAcquired)
            return;

        VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkClearValue clearValue;
        color_to_vk_clear_color(m_clearColor, clearValue.color);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain.extent();
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport vp{};
        vp.x = static_cast<float>(m_viewport.x);
        vp.y = static_cast<float>(m_viewport.y);
        vp.width = static_cast<float>(m_viewport.width);
        vp.height = static_cast<float>(m_viewport.height);
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &vp);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchain.extent();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        m_inRenderPass = true;
    }

    void VulkanBackend::end_render_pass() {
        if (!m_inRenderPass)
            return;

        VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);
        m_inRenderPass = false;
    }

    VkCommandBuffer VulkanBackend::current_command_buffer() {
        if (!m_frameAcquired || !m_inRenderPass)
            return VK_NULL_HANDLE;
        return m_commandBuffers[m_currentFrame];
    }

    bool VulkanBackend::create_default_pipeline() {
        auto vertSpirv = get_default_vertex_spirv();
        auto fragSpirv = get_default_fragment_spirv();
        if (vertSpirv.empty() || fragSpirv.empty())
            return false;

        std::vector<uint8_t> vertBytes(vertSpirv.size() * sizeof(uint32_t));
        std::memcpy(vertBytes.data(), vertSpirv.data(), vertBytes.size());
        std::vector<uint8_t> fragBytes(fragSpirv.size() * sizeof(uint32_t));
        std::memcpy(fragBytes.data(), fragSpirv.data(), fragBytes.size());

        ShaderSource vertSource(ShaderStage::Vertex, vertBytes);
        ShaderSource fragSource(ShaderStage::Fragment, fragBytes);

        auto vertShader = std::make_unique<VulkanShader>();
        auto fragShader = std::make_unique<VulkanShader>();
        if (!vertShader->create(m_device, vertSource) || !fragShader->create(m_device, fragSource))
            return false;

        VulkanPipelineCreateInfo pipeInfo{};
        pipeInfo.device = m_device;
        pipeInfo.renderPass = m_renderPass;
        pipeInfo.colorAttachmentFormat = m_swapchain.imageFormat();
        pipeInfo.vertexShader = vertShader.get();
        pipeInfo.fragmentShader = fragShader.get();
        pipeInfo.viewportExtent = m_swapchain.extent();
        pipeInfo.depthTest = m_depthTest;
        pipeInfo.depthWrite = m_depthWrite;
        pipeInfo.depthFunc = m_depthFunc;
        pipeInfo.blend = m_blend;
        pipeInfo.blendSrc = m_blendSrc;
        pipeInfo.blendDst = m_blendDst;
        pipeInfo.cullMode = m_cullMode;
        pipeInfo.windingOrder = m_windingOrder;

        auto pipeline = std::make_unique<VulkanPipeline>();
        if (!pipeline->create(pipeInfo))
            return false;

        RenderHandle vertHandle = m_nextShaderHandle++;
        RenderHandle fragHandle = m_nextShaderHandle++;
        m_shaders[vertHandle] = std::move(vertShader);
        m_shaders[fragHandle] = std::move(fragShader);

        RenderHandle pipeHandle = m_nextPipelineHandle++;
        m_pipelines[pipeHandle] = std::move(pipeline);
        m_pipelineToShader[pipeHandle] = fragHandle;
        m_boundPipeline = pipeHandle;
        return true;
    }

    void VulkanBackend::cleanup_swapchain() {
        for (auto fb : m_swapchainFramebuffers) {
            vkDestroyFramebuffer(m_device, fb, nullptr);
        }
        m_swapchainFramebuffers.clear();
        m_swapchain.destroy(m_device);
    }

    void VulkanBackend::recreate_swapchain() {
        vkDeviceWaitIdle(m_device);

        cleanup_swapchain();

        auto indices = find_queue_families(m_physicalDevice, m_surface);
        if (!m_swapchain.recreate(m_device, m_physicalDevice, m_surface, indices.graphics, indices.present, m_width,
                                  m_height)) {
            RIBBLE_LOG_ERROR("Failed to recreate Vulkan swapchain");
            return;
        }

        if (!create_framebuffers()) {
            RIBBLE_LOG_ERROR("Failed to recreate Vulkan framebuffers");
        }
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::begin_frame() {
        const VkFence fence = m_sync.inFlightFence(m_currentFrame);
        vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain.handle(), UINT64_MAX,
                                                m_sync.imageAvailable(m_currentFrame), VK_NULL_HANDLE,
                                                &m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return ribble::core::Ok();
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Failed to acquire swapchain image"));
        }

        m_frameAcquired = true;
        vkResetFences(m_device, 1, &fence);

        vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
        begin_render_pass();
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::end_frame() {
        if (!m_frameAcquired)
            return ribble::core::Ok();

        end_render_pass();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_sync.imageAvailable(m_currentFrame)};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

        VkSemaphore signalSemaphores[] = {m_sync.renderFinished(m_currentFrame)};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_sync.inFlightFence(m_currentFrame)) != VK_SUCCESS) {
            m_frameAcquired = false;
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Failed to submit Vulkan draw command"));
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        const VkSwapchainKHR swapchain = m_swapchain.handle();
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
        m_frameAcquired = false;
        m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Failed to present swapchain image"));
        }

        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_viewport(const Viewport &viewport) {
        m_viewport = viewport;
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_clear_color(const ribble::render::ColorRGBA &color) {
        m_clearColor = color;
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::clear() {
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::on_resize(int width, int height) {
        m_width = width;
        m_height = height;
        m_viewport = {0, 0, width, height};
        if (m_width > 0 && m_height > 0) {
            recreate_swapchain();
        }
        return ribble::core::Ok();
    }

    const char *VulkanBackend::backend_name() const { return m_context->backend_name(); }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    VulkanBackend::create_shader(const ShaderSource &source) {
        if (source.language != ShaderLanguage::SPIRV) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Vulkan requires SPIR-V shaders"));
        }
        auto shader = std::make_unique<VulkanShader>();
        if (!shader->create(m_device, source)) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Failed to create Vulkan shader module"));
        }
        RenderHandle h = m_nextShaderHandle++;
        m_shaders[h] = std::move(shader);
        return ribble::core::Ok(h);
    }
    void VulkanBackend::destroy_shader(RenderHandle h) { m_shaders.erase(h); }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    VulkanBackend::create_texture(int w, int h, TextureFormat fmt, const void *data) {
        auto tex = std::make_unique<VulkanTexture>();
        if (!tex->create(m_device, m_physicalDevice, m_graphicsQueue, m_commandPool, w, h, fmt, data)) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::TextureCreationFailure, "Failed to create Vulkan texture"));
        }
        RenderHandle handle = m_nextTextureHandle++;
        m_textures[handle] = std::move(tex);
        return ribble::core::Ok(handle);
    }
    void VulkanBackend::destroy_texture(RenderHandle h) { m_textures.erase(h); }

    ribble::core::Result<RenderHandle, RenderBackend::Failure>
    VulkanBackend::create_buffer(BufferType type, BufferUsage usage, size_t size, const void *data) {
        auto buf = std::make_unique<VulkanBuffer>();
        if (!buf->create(m_device, m_physicalDevice, type, usage, size, data, m_graphicsQueue, m_commandPool)) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::BufferCreationFailure, "Failed to create Vulkan buffer"));
        }
        RenderHandle h = m_nextBufferHandle++;
        m_buffers[h] = std::move(buf);
        return ribble::core::Ok(h);
    }
    void VulkanBackend::destroy_buffer(RenderHandle h) { m_buffers.erase(h); }

    ribble::core::Result<RenderHandle, RenderBackend::Failure> VulkanBackend::create_vertex_array() {
        RenderHandle h = m_nextVertexArrayHandle++;
        m_vertexArrays[h] = VulkanVertexArray{};
        return ribble::core::Ok(h);
    }
    void VulkanBackend::destroy_vertex_array(RenderHandle h) { m_vertexArrays.erase(h); }

    ribble::core::Result<RenderHandle, RenderBackend::Failure> VulkanBackend::create_framebuffer() {
        return ribble::core::Fail(
                RIBBLE_ERROR(Failure::FramebufferCreationFailure, "Offscreen framebuffers not yet implemented"));
    }
    void VulkanBackend::destroy_framebuffer(RenderHandle) {}

    ribble::core::Result<RenderHandle, RenderBackend::Failure> VulkanBackend::create_pipeline(RenderHandle shaderHandle) {
        auto it = m_shaders.find(shaderHandle);
        if (it == m_shaders.end()) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Invalid shader handle for pipeline"));
        }
        VulkanShader *shader = it->second.get();
        VulkanShader *vertShader = nullptr;
        VulkanShader *fragShader = nullptr;
        if (shader->stage() == ShaderStage::Vertex) {
            vertShader = shader;
            auto defFrag = get_default_fragment_spirv();
            if (!defFrag.empty()) {
                std::vector<uint8_t> fragBytes(defFrag.size() * sizeof(uint32_t));
                std::memcpy(fragBytes.data(), defFrag.data(), fragBytes.size());
                ShaderSource fragSrc(ShaderStage::Fragment, fragBytes);
                auto frag = std::make_unique<VulkanShader>();
                if (frag->create(m_device, fragSrc)) {
                    RenderHandle fh = m_nextShaderHandle++;
                    m_shaders[fh] = std::move(frag);
                    fragShader = m_shaders[fh].get();
                }
            }
        } else if (shader->stage() == ShaderStage::Fragment) {
            fragShader = shader;
            auto defVert = get_default_vertex_spirv();
            if (!defVert.empty()) {
                std::vector<uint8_t> vertBytes(defVert.size() * sizeof(uint32_t));
                std::memcpy(vertBytes.data(), defVert.data(), vertBytes.size());
                ShaderSource vertSrc(ShaderStage::Vertex, vertBytes);
                auto vert = std::make_unique<VulkanShader>();
                if (vert->create(m_device, vertSrc)) {
                    RenderHandle vh = m_nextShaderHandle++;
                    m_shaders[vh] = std::move(vert);
                    vertShader = m_shaders[vh].get();
                }
            }
        }
        if (!vertShader || !fragShader) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Need both vertex and fragment shaders"));
        }
        VulkanPipelineCreateInfo pipeInfo{};
        pipeInfo.device = m_device;
        pipeInfo.renderPass = m_renderPass;
        pipeInfo.colorAttachmentFormat = m_swapchain.imageFormat();
        pipeInfo.vertexShader = vertShader;
        pipeInfo.fragmentShader = fragShader;
        pipeInfo.viewportExtent = m_swapchain.extent();
        pipeInfo.depthTest = m_depthTest;
        pipeInfo.depthWrite = m_depthWrite;
        pipeInfo.depthFunc = m_depthFunc;
        pipeInfo.blend = m_blend;
        pipeInfo.blendSrc = m_blendSrc;
        pipeInfo.blendDst = m_blendDst;
        pipeInfo.cullMode = m_cullMode;
        pipeInfo.windingOrder = m_windingOrder;
        auto pipe = std::make_unique<VulkanPipeline>();
        if (!pipe->create(pipeInfo)) {
            return ribble::core::Fail(
                    RIBBLE_ERROR(Failure::ShaderCompilationFailure, "Failed to create Vulkan pipeline"));
        }
        RenderHandle ph = m_nextPipelineHandle++;
        m_pipelines[ph] = std::move(pipe);
        m_pipelineToShader[ph] = shaderHandle;
        return ribble::core::Ok(ph);
    }
    void VulkanBackend::destroy_pipeline(RenderHandle h) {
        m_pipelines.erase(h);
        m_pipelineToShader.erase(h);
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::bind_pipeline(RenderHandle h) {
        m_boundPipeline = h;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::bind_texture(RenderHandle, int) {
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::bind_buffer(RenderHandle h, BufferType type) {
        if (m_boundVertexArray == InvalidHandle)
            return ribble::core::Ok();
        auto it = m_vertexArrays.find(m_boundVertexArray);
        if (it == m_vertexArrays.end())
            return ribble::core::Ok();
        if (type == BufferType::Vertex)
            it->second.vertexBuffer = h;
        else if (type == BufferType::Index)
            it->second.indexBuffer = h;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::bind_vertex_array(RenderHandle h) {
        m_boundVertexArray = h;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::bind_framebuffer(RenderHandle h) {
        m_boundFramebuffer = h;
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::draw_indexed(RenderHandle vaoHandle, uint32_t indexCount, IndexType indexType,
                                uint32_t indexOffset, int32_t baseVertex, PrimitiveTopology topology) {
        VkCommandBuffer cmd = current_command_buffer();
        if (cmd == VK_NULL_HANDLE)
            return ribble::core::Ok();

        auto pipeIt = m_pipelines.find(m_boundPipeline);
        auto vaoIt = m_vertexArrays.find(vaoHandle);
        if (pipeIt == m_pipelines.end() || vaoIt == m_vertexArrays.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid pipeline or VAO"));

        RenderHandle vbHandle = vaoIt->second.vertexBuffer;
        RenderHandle ibHandle = vaoIt->second.indexBuffer;
        if (vbHandle == InvalidHandle || ibHandle == InvalidHandle)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "VAO has no vertex/index buffer"));

        auto vbIt = m_buffers.find(vbHandle);
        auto ibIt = m_buffers.find(ibHandle);
        if (vbIt == m_buffers.end() || ibIt == m_buffers.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid buffer"));

        VkBuffer vbuf = vbIt->second->handle();
        VkBuffer ibuf = ibIt->second->handle();
        VkDeviceSize voffset = 0;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeIt->second->handle());
        vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &voffset);
        vkCmdBindIndexBuffer(cmd, ibuf, 0, to_vk_index_type(indexType));

        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, 64, m_pushConstantMVP);
        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, m_pushConstantColor);

        vkCmdDrawIndexed(cmd, indexCount, 1, indexOffset, baseVertex, 0);
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::draw_arrays(RenderHandle vaoHandle, uint32_t vertexCount, uint32_t vertexOffset,
                               PrimitiveTopology topology) {
        VkCommandBuffer cmd = current_command_buffer();
        if (cmd == VK_NULL_HANDLE)
            return ribble::core::Ok();

        auto pipeIt = m_pipelines.find(m_boundPipeline);
        auto vaoIt = m_vertexArrays.find(vaoHandle);
        if (pipeIt == m_pipelines.end() || vaoIt == m_vertexArrays.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid pipeline or VAO"));

        RenderHandle vbHandle = vaoIt->second.vertexBuffer;
        if (vbHandle == InvalidHandle)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "VAO has no vertex buffer"));

        auto vbIt = m_buffers.find(vbHandle);
        if (vbIt == m_buffers.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid buffer"));

        VkBuffer vbuf = vbIt->second->handle();
        VkDeviceSize voffset = vertexOffset * 32;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeIt->second->handle());
        vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &voffset);
        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, 64, m_pushConstantMVP);
        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, m_pushConstantColor);

        vkCmdDraw(cmd, vertexCount, 1, 0, 0);
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::draw_instanced(RenderHandle vaoHandle, uint32_t indexCount, uint32_t instanceCount,
                                  IndexType indexType, uint32_t indexOffset, int32_t baseVertex,
                                  PrimitiveTopology topology) {
        if (indexCount == 0)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Indexed instanced requires indexCount > 0"));
        VkCommandBuffer cmd = current_command_buffer();
        if (cmd == VK_NULL_HANDLE)
            return ribble::core::Ok();

        auto pipeIt = m_pipelines.find(m_boundPipeline);
        auto vaoIt = m_vertexArrays.find(vaoHandle);
        if (pipeIt == m_pipelines.end() || vaoIt == m_vertexArrays.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid pipeline or VAO"));

        RenderHandle vbHandle = vaoIt->second.vertexBuffer;
        RenderHandle ibHandle = vaoIt->second.indexBuffer;
        if (vbHandle == InvalidHandle || ibHandle == InvalidHandle)
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "VAO has no vertex/index buffer"));

        auto vbIt = m_buffers.find(vbHandle);
        auto ibIt = m_buffers.find(ibHandle);
        if (vbIt == m_buffers.end() || ibIt == m_buffers.end())
            return ribble::core::Fail(RIBBLE_ERROR(Failure::DrawFailure, "Invalid buffer"));

        VkBuffer vbuf = vbIt->second->handle();
        VkBuffer ibuf = ibIt->second->handle();
        VkDeviceSize voffset = 0;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeIt->second->handle());
        vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &voffset);
        vkCmdBindIndexBuffer(cmd, ibuf, 0, to_vk_index_type(indexType));
        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, 64, m_pushConstantMVP);
        vkCmdPushConstants(cmd, pipeIt->second->layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, m_pushConstantColor);

        vkCmdDrawIndexed(cmd, indexCount, instanceCount, indexOffset, baseVertex, 0);
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_depth_test(bool e) {
        m_depthTest = e;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_depth_write(bool e) {
        m_depthWrite = e;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_depth_func(DepthFunc f) {
        m_depthFunc = f;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_blend(bool e) {
        m_blend = e;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_blend_func(BlendFactor src, BlendFactor dst) {
        m_blendSrc = src;
        m_blendDst = dst;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_blend_op(BlendOp) {
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_cull_mode(CullMode m) {
        m_cullMode = m;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_winding_order(WindingOrder o) {
        m_windingOrder = o;
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure> VulkanBackend::set_program_point_size(bool) {
        return ribble::core::Ok();
    }

    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, int v) {
        if (name == "color" || name == "u_color") {
            m_pushConstantColor[0] = static_cast<float>(v);
            m_pushConstantColor[1] = static_cast<float>(v);
            m_pushConstantColor[2] = static_cast<float>(v);
            m_pushConstantColor[3] = 1.0f;
        }
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, float v) {
        if (name == "color" || name == "u_color" || name == "color.r") {
            m_pushConstantColor[0] = v;
            m_pushConstantColor[1] = v;
            m_pushConstantColor[2] = v;
            m_pushConstantColor[3] = 1.0f;
        }
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, float x, float y) {
        if (name == "color" || name == "u_color") {
            m_pushConstantColor[0] = x;
            m_pushConstantColor[1] = y;
            m_pushConstantColor[2] = 0;
            m_pushConstantColor[3] = 1.0f;
        }
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, float x, float y, float z) {
        if (name == "color" || name == "u_color") {
            m_pushConstantColor[0] = x;
            m_pushConstantColor[1] = y;
            m_pushConstantColor[2] = z;
            m_pushConstantColor[3] = 1.0f;
        }
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, float x, float y, float z, float w) {
        if (name == "color" || name == "u_color") {
            m_pushConstantColor[0] = x;
            m_pushConstantColor[1] = y;
            m_pushConstantColor[2] = z;
            m_pushConstantColor[3] = w;
        }
        return ribble::core::Ok();
    }
    ribble::core::Result<void, RenderBackend::Failure>
    VulkanBackend::set_uniform(RenderHandle, const std::string &name, const float *mat, bool transpose) {
        if (name == "mvp" || name == "u_mvp" || name == "modelViewProjection") {
            std::memcpy(m_pushConstantMVP, mat, 16 * sizeof(float));
        }
        return ribble::core::Ok();
    }

} // namespace backend
