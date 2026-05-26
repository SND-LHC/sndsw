#include "scene/Mesh.hpp"

#include <iostream>

#include <glad/glad.h>

using namespace std;
using namespace glm;

namespace snd3D {

    Mesh::Mesh(string _name, const std::shared_ptr<GpuMesh>& _gpuMesh) {
        this->name = _name;
        this->gpuMesh = _gpuMesh;
        this->modelMatrix = glm::mat4(1.0f);
        this->anchor = this->gpuMesh->getAnchor();
    }

    void Mesh::updateGlobalModelMatrix(const glm::mat4& parentModelMatrix) {
        this->modelMatrix = parentModelMatrix;
        this->anchor = glm::vec3(this->modelMatrix * glm::vec4(this->gpuMesh->getAnchor(), 1.0f));
    }

    void Mesh::setMaterial(const shared_ptr<Material>& _material) {
        this->material = _material;
    }

    void Mesh::setActive(bool value) {
        this->active = value;
    }

    void Mesh::render(bool showAnchor, Shader* shader) {
        if (this->active) {
            if (shader == nullptr) {
                cerr << "ATTENTION!!! Shader not set" << endl;
                return;
            }
            shader->bindLocalUniforms(this->modelMatrix, this->material.get());
            this->gpuMesh->render(showAnchor);
        }
    }

    const glm::vec3& Mesh::getAnchor() {
        return this->anchor;
    }
}
