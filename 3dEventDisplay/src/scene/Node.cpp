#include "scene/Node.hpp"

namespace snd3D {

    Node::Node(const aiScene* _scene, aiNode* _node, std::vector<std::shared_ptr<GpuMesh>>& _meshes, std::vector<std::shared_ptr<Material>>& _materials) {

        aiMatrix4x4 matrix = _node->mTransformation;

        this->localModelMatrix = glm::mat4(
            matrix.a1, matrix.b1, matrix.c1, matrix.d1,
            matrix.a2, matrix.b2, matrix.c2, matrix.d2,
            matrix.a3, matrix.b3, matrix.c3, matrix.d3,
            matrix.a4, matrix.b4, matrix.c4, matrix.d4
        );

        this->name = std::string(_node->mName.C_Str());

        // Create a mesh for all the meshes of the node and assign material
        for (unsigned int i = 0; i < _node->mNumMeshes; i++) {
            unsigned int meshId = _node->mMeshes[i];
            aiMesh* assimpMesh = _scene->mMeshes[meshId];

            std::string _name = std::string(assimpMesh->mName.C_Str());
            if (_name.empty()) {
                _name = "Unnamed_Mesh_" + std::to_string(meshId);
            }

            std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(_name, _meshes[meshId]);
            mesh->setMaterial(_materials[assimpMesh->mMaterialIndex]);
            this->meshes.push_back(std::move(mesh));
        }

        // Create children nodes
        this->childrenNode.reserve(_node->mNumChildren);
        for (unsigned int i = 0; i < _node->mNumChildren; i++) {
            this->childrenNode.push_back(std::make_unique<Node>(_scene, _node->mChildren[i], _meshes, _materials));
        }
    }

    Node::Node(std::string _name, glm::mat4 modelMatrix) {
        this->name = _name;
        this->localModelMatrix = modelMatrix;
        this->globalModelMatrix = glm::mat4(1.0f);
    }

    void Node::addChild(Node* node) {
        this->childrenNode.push_back(std::unique_ptr<Node>(node));
        node->updateGlobalModelMatrix(this->globalModelMatrix);
    }

    void Node::addMesh(Mesh* mesh) {
        this->meshes.push_back(std::unique_ptr<Mesh>(mesh));
    }

    void Node::setGlobalActive(bool value) {
        this->active = value;

        for (auto& node : this->childrenNode) {
            node->setGlobalActive(value);
        }

        for (auto& mesh : this->meshes) {
            mesh->setActive(value);
        }
    }

    void Node::updateGlobalModelMatrix(const glm::mat4& parentModelMatrix) {
        this->globalModelMatrix = parentModelMatrix * this->localModelMatrix;

        for (auto& node : this->childrenNode) {
            node->updateGlobalModelMatrix(this->globalModelMatrix);
        }
    }

    void Node::render(const glm::mat4& parentModelMatrix, bool showAnchor, Shader* shader) {
        glm::mat4 modelMatrix = parentModelMatrix * this->localModelMatrix;

        for (auto& node : this->childrenNode) {
            node->render(modelMatrix, showAnchor, shader);
        }

        for (auto& mesh : this->meshes) {
            mesh->render(modelMatrix, showAnchor, shader);
        }
    }

    void Node::render(bool showAnchor, Shader* shader) {

        if (this->active) {
            for (auto& node : this->childrenNode) {
                node->render(showAnchor, shader);
            }

            for (auto& mesh : this->meshes) {
                mesh->render(this->globalModelMatrix, showAnchor, shader);
            }
        }
    }
}
