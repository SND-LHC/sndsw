#include "scene/Mesh.hpp"

#include <iostream>

#include <glad/glad.h>

using namespace std;
using namespace glm;

namespace snd3D {

    Mesh::Mesh(string _name, const std::shared_ptr<GpuMesh>& _gpuMesh) {
        this->name = _name;
        this->gpuMesh = _gpuMesh;
    }

    void Mesh::setMaterial(const shared_ptr<Material>& _material) {
        this->material = _material;
    }

    void Mesh::setActive(bool value) {
        this->active = value;
    }

    void Mesh::render(const glm::mat4& modelMatrix, bool showAnchor, Shader* shader) {
        if (this->active) {
            if (shader == nullptr) {
                cerr << "ATTENTION!!! Shader not set" << endl;
                return;
            }
            shader->bindLocalUniforms(modelMatrix, this->material.get());
            this->gpuMesh->render(showAnchor);
        }
    }
}
