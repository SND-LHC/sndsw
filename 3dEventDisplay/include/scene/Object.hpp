#pragma once

#include <memory>
#include <vector>

#include <assimp/scene.h>
#include <glm/fwd.hpp>

#include "scene/Node.hpp"
#include "scene/PointLight.hpp"
#include "rendering/engine/Shader.hpp"

namespace snd3D {

    class Viewport; // Forward declaration to avoid circular inclusion

    class Object {

        friend class Gui; // Needs access to show object propreties

        public:
            Object(const aiScene* _scene, std::vector<std::shared_ptr<GpuMesh>>& _meshes, std::vector<std::shared_ptr<Material>>& _materials);
            Object(Node* _rootNode);
            void setShader(const std::shared_ptr<Shader>& _shader);
            void updateModelMatrix(const glm::mat4& _modelMatrix);
            void sortMeshes(glm::vec3 point);
            void render(const Viewport& viewport, bool showAnchor, const std::vector<std::unique_ptr<PointLight>>& lights = {}, const float edgeAlphaValue = 1, const float faceAlphaValue = 1, const float edgeThickness = 1);
            void renderBuffered(const Viewport& viewport, bool showAnchor, const std::vector<std::unique_ptr<PointLight>>& lights = {}, const float edgeAlphaValue = 1, const float faceAlphaValue = 1, const float edgeThickness = 1);
            void setGlobalActive(bool value);

        private:
            std::unique_ptr<Node> rootNode;
            std::shared_ptr<Shader> shader;
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            bool active = true;

            std::vector<Mesh*> meshes; // Contains the reference to all the Meshes
    };
}
