#pragma once

#include <string>

#include <assimp/material.h>
#include <glm/glm.hpp>

namespace snd3D {
    class Material {
        public:
            Material(const aiMaterial* material);
            Material(glm::vec3 _baseColor);
            glm::vec3 getBaseColor();
            glm::vec3 getAmbient();
            glm::vec3 getDiffuse();
            glm::vec3 getSpecular();
            float getShininess();

        private:
            std::string name;
            float alpha;
            glm::vec3 baseColor;
            glm::vec3 ambient;
            glm::vec3 diffuse;
            glm::vec3 specular;
            float shininess;
    };
}
