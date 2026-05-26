#pragma once

#include <memory>
#include <vector>
#include <string>

#include <glm/fwd.hpp>
#include <assimp/scene.h>

#include "scene/Mesh.hpp"
#include "rendering/engine/GpuMesh.hpp"
#include "rendering/engine/Shader.hpp"
#include "rendering/engine/Material.hpp"

namespace snd3D {
    class Node {

        friend class Gui; // Needs access to show and edit node propreties

        public:
            Node(const aiScene* _scene, aiNode* _node, std::vector<std::shared_ptr<GpuMesh>>& _meshes, std::vector<std::shared_ptr<Material>>& _materials);
            Node(std::string _name, glm::mat4 modelMatrix = glm::mat4(1.0f));
            void addChild(Node* child);
            void addMesh(Mesh* mesh);
            void setGlobalActive(bool value);
            void updateGlobalModelMatrix(const glm::mat4& parentModelMatrix);
            void render(bool showAnchor, Shader* shader); // Use the buffered model matrix
            void getMeshes(std::vector<Mesh*>& outMeshes);

        private:
            std::string name;
            std::vector<std::unique_ptr<Node>> childrenNode;
            std::vector<std::unique_ptr<Mesh>> meshes;
            glm::mat4 localModelMatrix;
            glm::mat4 globalModelMatrix;
            bool active = true;
    };
}
