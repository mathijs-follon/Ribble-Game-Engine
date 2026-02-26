#include "ribble/render/renderer.h"

#include <glm/gtc/type_ptr.hpp>

#include "../../backend/common/backend_types.h"
#include "../../backend/common/window_events.h"
#include "../../backend/common/render_backend.h"
#include "../../backend/render/opengl/opengl_backend.h"
#include "../../backend/render/opengl/opengl_context_sdl3.h"
#include "opengl_context_glfw.h"
#ifdef RIBBLE_HAS_X11
#include "opengl_context_x11.h"
#endif
#ifdef RIBBLE_HAS_WAYLAND
#include "opengl_context_wayland.h"
#endif
#ifdef _WIN32
#include "opengl_context_win32.h"
#endif
#ifdef RIBBLE_HAS_VULKAN
#include "../../backend/render/vulkan/vulkan_backend.h"
#include "../../backend/render/vulkan/vulkan_context_glfw.h"
#include "../../backend/render/vulkan/vulkan_context_sdl3.h"
#endif

#include "ribble/core/logger.h"
#include "ribble/scene/node.h"
#include "ribble/window/window.h"

#include <algorithm>
#include <vector>

namespace ribble::render {

    namespace {

        static std::unique_ptr<backend::RenderBackend> create_backend(backend::WindowBackendType windowType,
                                                                     backend::RenderBackendType renderType) {
            using W = backend::WindowBackendType;
            using R = backend::RenderBackendType;

            switch (renderType) {
                case R::OpenGL: {
                    std::unique_ptr<backend::OpenGLContext> ctx;
                    switch (windowType) {
                        case W::SDL3:
                            ctx = std::make_unique<backend::OpenGLContextSDL3>();
                            break;
                        case W::GLFW:
                            ctx = std::make_unique<backend::OpenGLContextGLFW>();
                            break;
#ifdef RIBBLE_HAS_X11
                        case W::X11:
                            ctx = std::make_unique<backend::OpenGLContextX11>();
                            break;
#endif
#ifdef RIBBLE_HAS_WAYLAND
                        case W::Wayland:
                            ctx = std::make_unique<backend::OpenGLContextWayland>();
                            break;
#endif
#ifdef _WIN32
                        case W::Win32:
                            ctx = std::make_unique<backend::OpenGLContextWin32>();
                            break;
#endif
                        default:
                            RIBBLE_LOG_ERROR("Window backend {} not supported by OpenGL", static_cast<int>(windowType));
                            return nullptr;
                    }
                    return std::make_unique<backend::OpenGLBackend>(std::move(ctx));
                }

                case R::Vulkan: {
#ifdef RIBBLE_HAS_VULKAN
                    std::unique_ptr<backend::VulkanContext> vkCtx;
                    switch (windowType) {
                        case W::SDL3:
                            vkCtx = std::make_unique<backend::VulkanContextSDL3>();
                            break;
                        case W::GLFW:
                            vkCtx = std::make_unique<backend::VulkanContextGLFW>();
                            break;
                        default:
                            RIBBLE_LOG_ERROR("Vulkan requires SDL3 or GLFW window backend");
                            return nullptr;
                    }
                    return std::make_unique<backend::VulkanBackend>(std::move(vkCtx));
#else
                    RIBBLE_LOG_ERROR("Vulkan backend not available");
                    return nullptr;
#endif
                }

                case R::DirectX12:
                case R::Metal:
                    RIBBLE_LOG_ERROR("Render backend {} not implemented", static_cast<int>(renderType));
                    return nullptr;

                default:
                    RIBBLE_LOG_ERROR("Unknown render backend {}", static_cast<int>(renderType));
                    return nullptr;
            }
        }

        void collect_renderables(scene::Node *node, const glm::mat4 &parentWorld,
                                 std::vector<std::pair<glm::mat4, const scene::Renderable *>> &out) {
            if (!node)
                return;

            glm::mat4 world = parentWorld * node->local_matrix();

            if (node->has_renderable()) {
                const scene::Renderable *r = node->renderable();
                if (r && (r->vertexArrayHandle != 0 || r->vertexCount > 0)) {
                    out.emplace_back(world, r);
                }
            }

            for (const auto &child : node->children()) {
                collect_renderables(child.get(), world, out);
            }
        }

    } // namespace

    struct Renderer::Impl {
        backend::WindowBackendType windowType;
        backend::RenderBackendType renderType;
        std::unique_ptr<backend::RenderBackend> backend;
        ColorRGBA clearColor{0.1f, 0.1f, 0.1f, 1.0f};
    };

    Renderer::Renderer(backend::WindowBackendType windowType, backend::RenderBackendType renderType)
        : m_impl(std::make_unique<Impl>()) {
        m_impl->windowType = windowType;
        m_impl->renderType = renderType;
        m_impl->backend = create_backend(windowType, renderType);
    }

    Renderer::~Renderer() {
        if (m_impl && m_impl->backend && m_impl->backend->is_initialized()) {
            shutdown();
        }
    }

    core::Result<void, RendererFailure> Renderer::initialize(ribble::window::WindowContext &windowContext, int width,
                                                             int height) {
        if (!m_impl->backend) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "No render backend"));
        }

        auto result = m_impl->backend->initialize(windowContext, width, height);
        if (!result) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::InitializationFailure, "Backend init failed"));
        }

        return core::Ok();
    }

    void Renderer::shutdown() {
        if (m_impl->backend) {
            m_impl->backend->shutdown();
        }
    }

    bool Renderer::is_initialized() const {
        return m_impl->backend && m_impl->backend->is_initialized();
    }

    core::Result<void, RendererFailure> Renderer::draw_scene(scene::Scene *scene, const CameraView &camera) {
        if (!m_impl->backend || !m_impl->backend->is_initialized()) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "Renderer not initialized"));
        }

        backend::RenderBackend &backend = *m_impl->backend;

        if (auto r = backend.begin_frame(); !r) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "begin_frame failed"));
        }

        backend::Viewport vp{camera.viewportX, camera.viewportY,
                             static_cast<int>(camera.viewportWidth),
                             static_cast<int>(camera.viewportHeight)};
        backend.set_viewport(vp);
        backend.set_clear_color(m_impl->clearColor);
        backend.clear();

        // Collect all renderables from the scene tree (if scene is provided)
        std::vector<std::pair<glm::mat4, const scene::Renderable *>> renderables;
        if (scene && scene->root()) {
            collect_renderables(scene->root(), glm::mat4(1), renderables);
        }

        // Sort by pipeline for batching (minimize state changes)
        std::sort(renderables.begin(), renderables.end(),
                  [](const auto &a, const auto &b) {
                      return a.second->pipelineHandle < b.second->pipelineHandle;
                  });

        glm::mat4 vpMatrix = camera.view_projection();

        for (const auto &[world, r] : renderables) {
            glm::mat4 mvp = vpMatrix * world;

            if (r->pipelineHandle != 0) {
                backend.bind_pipeline(static_cast<backend::RenderHandle>(r->pipelineHandle));
            }
            if (r->vertexArrayHandle != 0) {
                backend.bind_vertex_array(static_cast<backend::RenderHandle>(r->vertexArrayHandle));
            }

            backend.set_uniform(static_cast<backend::RenderHandle>(r->pipelineHandle), "mvp",
                               glm::value_ptr(mvp), false);
            backend.set_uniform(static_cast<backend::RenderHandle>(r->pipelineHandle), "color",
                               r->color[0], r->color[1], r->color[2], r->color[3]);

            if (r->indexed && r->indexCount > 0) {
                auto drawResult = backend.draw_indexed(
                        static_cast<backend::RenderHandle>(r->vertexArrayHandle),
                        r->indexCount, backend::IndexType::UInt32, 0, 0,
                        backend::PrimitiveTopology::Triangles);
                if (!drawResult) {
                    backend.end_frame();
                    return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "draw_indexed failed"));
                }
            } else if (r->vertexCount > 0) {
                auto drawResult = backend.draw_arrays(
                        static_cast<backend::RenderHandle>(r->vertexArrayHandle),
                        r->vertexCount, 0, backend::PrimitiveTopology::Triangles);
                if (!drawResult) {
                    backend.end_frame();
                    return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "draw_arrays failed"));
                }
            }
        }

        if (auto r = backend.end_frame(); !r) {
            return core::Fail(RIBBLE_ERROR(RendererFailure::DrawFailure, "end_frame failed"));
        }

        return core::Ok();
    }

    void Renderer::set_clear_color(const ColorRGBA &color) {
        m_impl->clearColor = color;
    }

    void Renderer::set_clear_color(float r, float g, float b, float a) {
        m_impl->clearColor = ColorRGBA{r, g, b, a};
    }

    backend::RenderBackend &Renderer::backend() {
        return *m_impl->backend;
    }

    const backend::RenderBackend &Renderer::backend() const {
        return *m_impl->backend;
    }

} // namespace ribble::render
