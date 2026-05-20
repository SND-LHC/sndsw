#include "scene/Object.hpp"

#include <vector>

#include <assimp/color4.h>
#include <glm/ext/matrix_transform.hpp>

#include "rendering/engine/Material.hpp"
#include "rendering/Viewport.hpp"
#include "scene/Mesh.hpp"

namespace snd3D {

    Object::Object(const aiScene* _scene, std::vector<std::shared_ptr<GpuMesh>>& _meshes, std::vector<std::shared_ptr<Material>>& _materials) {
        // Recursively start creating nodes and store buffered model matrices
        this->rootNode = std::make_unique<Node>(_scene, _scene->mRootNode, _meshes, _materials);
        this->rootNode->updateGlobalModelMatrix(this->modelMatrix);
    }

    Object::Object(Node* _rootNode) {
        this->rootNode = std::unique_ptr<Node>(_rootNode);
    }

    void Object::setShader(const std::shared_ptr<Shader>& _shader) {
        this->shader = _shader;
    }

    void Object::updateModelMatrix(const glm::mat4& _modelMatrix) {
        this->modelMatrix = _modelMatrix;
        this->rootNode->updateGlobalModelMatrix(this->modelMatrix);
    }

    void Object::render(const Viewport& viewport, bool showAnchor, const std::vector<std::unique_ptr<PointLight>>& lights, const float edgeAlphaValue, const float faceAlphaValue, const float edgeThickness) {
        if (this->active) {
            this->shader->use();
            this->shader->bindGlobalUniforms(viewport.getViewMatrix(), viewport.getProjectionMatrix(), viewport.getCameraPosition(), lights, edgeAlphaValue, faceAlphaValue, edgeThickness);
            // Using the render mode without passing model matrix beacuse we calculated it in the object creation
            this->rootNode->render(showAnchor, this->shader.get());
        }
    }

    void Object::setGlobalActive(bool value) {
        this->active = value;
        this->rootNode->setGlobalActive(value);
    }
}
