#include "rendering/engine/Material.hpp"

#include <assimp/material.h>
#include <assimp/GltfMaterial.h>
#include <assimp/types.h>

namespace snd3D {

    Material::Material(const aiMaterial* material) {
        aiString _name;
        aiColor4D _baseColor(0.5f, 0.5f, 0.5f, 1.0f);
        float metallic = 0, roughness = 1;

        if (material->Get(AI_MATKEY_NAME, _name) != AI_SUCCESS) {
            this->name = "NoNameMaterial";
        } else {
            this->name = std::string(_name.C_Str());
        }

        material->Get(AI_MATKEY_BASE_COLOR, _baseColor);
        material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

        glm::vec3 color = glm::vec3(_baseColor.r, _baseColor.g, _baseColor.b);
        this->alpha = _baseColor.a;

        this->baseColor = color;
        this->ambient = color * 0.1f;
        this->diffuse = color * (1.0f - metallic);
        this->specular = glm::mix(glm::vec3(0.04f), color, metallic);
        this->shininess = (1.0f - roughness) * 128.0f;
    }

    Material::Material(glm::vec3 _baseColor) {
        this->baseColor = _baseColor;
        this->ambient = glm::vec4(_baseColor * 0.2f, 1.0f);
        this->diffuse = glm::vec4(_baseColor * 0.8f, 1.0f);
        this->specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        this->shininess = 32.0f;
    }

    glm::vec3 Material::getBaseColor() {
        return this->baseColor;
    }

    glm::vec3 Material::getAmbient() {
        return this->ambient;
    }

    glm::vec3 Material::getDiffuse() {
        return this->diffuse;
    }

    glm::vec3 Material::getSpecular() {
        return this->specular;
    }

    float Material::getShininess() {
        return this->shininess;
    }
}
