#include "scene/Object.hpp"

#include <vector>
#include <algorithm>

#include <assimp/color4.h>
#define GLM_ENABLE_EXPERIMENTAL // Enable experimental features, used for glm::distance2
#include <glm/gtx/norm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "rendering/engine/Material.hpp"
#include "rendering/Viewport.hpp"
#include "scene/Mesh.hpp"

namespace snd3D {

    Object::Object(const aiScene* _scene, std::vector<std::shared_ptr<GpuMesh>>& _meshes, std::vector<std::shared_ptr<Material>>& _materials) {
        // Recursively start creating nodes and store buffered model matrices
        this->rootNode = std::make_unique<Node>(_scene, _scene->mRootNode, _meshes, _materials);
        this->rootNode->updateGlobalModelMatrix(this->modelMatrix);
        this->rootNode->getMeshes(this->meshes);
    }

    Object::Object(Node* _rootNode) {
        this->rootNode = std::unique_ptr<Node>(_rootNode);
        this->rootNode->getMeshes(this->meshes);
    }

    void Object::setShader(const std::shared_ptr<Shader>& _shader) {
        this->shader = _shader;
    }

    void Object::updateModelMatrix(const glm::mat4& _modelMatrix) {
        this->modelMatrix = _modelMatrix;
        this->rootNode->updateGlobalModelMatrix(this->modelMatrix);
    }

    void Object::sortMeshes(glm::vec3 point) {

        std::sort(this->meshes.begin(), this->meshes.end(), [point](Mesh* a, Mesh* b) {
            float distA = glm::distance2(a->getAnchor(), point);
            float distB = glm::distance2(b->getAnchor(), point);

            return distA > distB;
        });
    
    }

    void Object::render(const Viewport& viewport, bool showAnchor, const float ambientLight, const std::vector<std::unique_ptr<PointLight>>& lights, const float edgeAlphaValue, const float faceAlphaValue, const float edgeThickness) {
        if (this->active) {
            this->shader->use();
            this->shader->bindGlobalUniforms(viewport.getViewMatrix(), viewport.getProjectionMatrix(), ambientLight, lights, edgeAlphaValue, faceAlphaValue, edgeThickness);
            // Using the render mode without passing model matrix beacuse we calculated it in the object creation
            this->rootNode->render(showAnchor, this->shader.get());
        }
    }

    void Object::renderBuffered(const Viewport& viewport, bool showAnchor, const float ambientLight, const std::vector<std::unique_ptr<PointLight>>& lights, const float edgeAlphaValue, const float faceAlphaValue, const float edgeThickness) {
        if (this->active) {
            this->shader->use();
            this->shader->bindGlobalUniforms(viewport.getViewMatrix(), viewport.getProjectionMatrix(), ambientLight, lights, edgeAlphaValue, faceAlphaValue, edgeThickness);

            for (Mesh* mesh: this->meshes) {
                mesh->render(showAnchor, this->shader.get());
            }
        }
    }

    void Object::setGlobalActive(bool value) {
        this->active = value;
        this->rootNode->setGlobalActive(value);
    }
}
