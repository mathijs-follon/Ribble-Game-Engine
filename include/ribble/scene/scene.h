#pragma once

#include <memory>

#include "ribble/scene/node.h"

namespace ribble::scene {

    /// Scene - root container for the scene tree. Holds the root node and all descendants.
    class Scene {
    public:
        Scene();
        ~Scene() = default;

        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;

        [[nodiscard]] Node *root() { return m_root.get(); }
        [[nodiscard]] const Node *root() const { return m_root.get(); }

        /// Add a child to the root. Returns the added node (owned by the scene).
        Node *add_node(std::unique_ptr<Node> node);

        /// Add a child to a specific parent.
        Node *add_node(Node *parent, std::unique_ptr<Node> node);

    private:
        std::unique_ptr<Node> m_root;
    };

} // namespace ribble::scene
