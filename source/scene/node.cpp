#include "ribble/scene/node.h"

namespace ribble::scene {

    glm::mat4 Node::local_matrix() const {
        glm::mat4 t = glm::translate(glm::mat4(1), m_position);
        glm::mat4 r = glm::mat4_cast(m_rotation);
        glm::mat4 s = glm::scale(glm::mat4(1), m_scale);
        return t * r * s;
    }

    glm::mat4 Node::world_matrix() {
        if (m_dirtyWorld) {
            if (m_parent) {
                m_worldMatrix = m_parent->world_matrix() * local_matrix();
            } else {
                m_worldMatrix = local_matrix();
            }
            m_dirtyWorld = false;
        }
        return m_worldMatrix;
    }

    void Node::invalidate_world() {
        m_dirtyWorld = true;
        for (auto &child : m_children) {
            child->invalidate_world();
        }
    }

    Node *Node::add_child(std::unique_ptr<Node> node) {
        if (!node)
            return nullptr;
        Node *ptr = node.get();
        node->set_parent(this);
        m_children.push_back(std::move(node));
        return ptr;
    }

    std::unique_ptr<Node> Node::remove_child(Node *node) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if (it->get() == node) {
                std::unique_ptr<Node> extracted = std::move(*it);
                m_children.erase(it);
                extracted->set_parent(nullptr);
                return extracted;
            }
        }
        return nullptr;
    }

} // namespace ribble::scene
