#pragma once

#include <memory>

#include <assimp/scene.h>
#include <glm/fwd.hpp>

#include "scene/Node.hpp"
#include "rendering/engine/Shader.hpp"

namespace snd3D {

    class Viewport; // Forward declaration to avoid circular inclusion

    class Object {

        friend class Gui; // Needs access to show object propreties

        public:
            Object(const aiScene* _scene);
            Object(Node* _rootNode);
            void setShader(const std::shared_ptr<Shader>& _shader);
            void updateModelMatrix(const glm::mat4& _modelMatrix);
            void render(const Viewport& viewport, bool showAnchor, const float edgeAlphaValue = 1, const float faceAlphaValue = 1, const float edgeThickness = 1);
            void setGlobalActive(bool value);

        private:
            std::unique_ptr<Node> rootNode;
            std::shared_ptr<Shader> shader;
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            bool active = true;
    };
}
