#include "ribble/scene/scene.h"

namespace ribble::scene {

    Scene::Scene() : m_root(std::make_unique<Node>("Root")) {}

    Node *Scene::add_node(std::unique_ptr<Node> node) {
        return add_node(m_root.get(), std::move(node));
    }

    Node *Scene::add_node(Node *parent, std::unique_ptr<Node> node) {
        if (!parent || !node)
            return nullptr;
        return parent->add_child(std::move(node));
    }

} // namespace ribble::scene
