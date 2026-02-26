#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ribble::scene {

    /// Renderable data attached to a node - describes what to draw (mesh, pipeline, etc.)
    struct Renderable {
        uint32_t vertexArrayHandle{0};
        uint32_t pipelineHandle{0};
        uint32_t indexCount{0};
        uint32_t vertexCount{0};
        bool indexed{true};
        float color[4]{1.0f, 1.0f, 1.0f, 1.0f};
    };

    /// Node in the scene tree. Has transform, children, and optional renderable component.
    class Node {
    public:
        Node() = default;
        explicit Node(std::string name) : m_name(std::move(name)) {}

        [[nodiscard]] const std::string &name() const { return m_name; }
        void set_name(const std::string &name) { m_name = name; }

        [[nodiscard]] const glm::vec3 &position() const { return m_position; }
        void set_position(const glm::vec3 &p) { m_position = p; m_dirtyWorld = true; }

        [[nodiscard]] const glm::vec3 &scale() const { return m_scale; }
        void set_scale(const glm::vec3 &s) { m_scale = s; m_dirtyWorld = true; }

        [[nodiscard]] const glm::quat &rotation() const { return m_rotation; }
        void set_rotation(const glm::quat &q) { m_rotation = q; m_dirtyWorld = true; }

        [[nodiscard]] glm::mat4 local_matrix() const;

        /// World matrix (cached, invalidated when transform changes)
        [[nodiscard]] glm::mat4 world_matrix();

        Node *parent() { return m_parent; }
        [[nodiscard]] const Node *parent() const { return m_parent; }

        void set_parent(Node *p) { m_parent = p; m_dirtyWorld = true; }

        [[nodiscard]] const std::vector<std::unique_ptr<Node>> &children() const { return m_children; }

        Node *add_child(std::unique_ptr<Node> node);
        std::unique_ptr<Node> remove_child(Node *node);

        /// Optional renderable component. If set, this node is drawn when in view.
        [[nodiscard]] Renderable *renderable() { return m_renderable ? &*m_renderable : nullptr; }
        [[nodiscard]] const Renderable *renderable() const {
            return m_renderable ? &*m_renderable : nullptr;
        }

        void set_renderable(Renderable r) { m_renderable = std::move(r); }
        void clear_renderable() { m_renderable.reset(); }
        [[nodiscard]] bool has_renderable() const { return m_renderable.has_value(); }

    private:
        std::string m_name;
        glm::vec3 m_position{0};
        glm::vec3 m_scale{1};
        glm::quat m_rotation{1, 0, 0, 0};

        Node *m_parent{nullptr};
        std::vector<std::unique_ptr<Node>> m_children;

        std::optional<Renderable> m_renderable;

        mutable glm::mat4 m_worldMatrix{1};
        mutable bool m_dirtyWorld{true};

        void invalidate_world();
    };

} // namespace ribble::scene
