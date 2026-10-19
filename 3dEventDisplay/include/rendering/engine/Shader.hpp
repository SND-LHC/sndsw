/**
 * @file Shader.hpp
 * @brief Manages OpenGL shader programs and uniform variable settings.
 *
 * Shader wraps OpenGL shader program compilation and provides methods to set
 * transformation matrices, material properties, lighting parameters, and texture
 * samplers. Supports optional geometry shaders for advanced rendering techniques.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <glm/fwd.hpp>

#include "core/Constants.hpp"
#include "rendering/engine/Material.hpp"
#include "scene/PointLight.hpp"

namespace snd3D {
    class Shader {
        public:
            Shader(std::string _name, std::string _vertexShader, std::string _fragmentShader, std::string _geometryShader = "");
            ~Shader();
            std::string getName();
            GLuint getProgramId();
            void use();
            void bindGlobalUniforms(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const float ambientLight = 0, const std::vector<std::unique_ptr<PointLight>>& lights = {}, const float edgeAlphaValue = 0.5f, const float faceAlphaValue = 0.5f, const float edgeThickness = 0.25f);
            void bindLocalUniforms(const glm::mat4& modelMatrix, Material* material);

        private:
            std::string name;
            GLuint programId;

            // Cached handles to shader uniform variables to avoid repeated lookups, -1 if not used by the shader
            GLint uniform_Projection = -1;
            GLint uniform_Model = -1;
            GLint uniform_View = -1;
            GLint uniform_MaterialBaseColor = -1;
            GLint uniform_MaterialAmbient = -1;
            GLint uniform_MaterialDiffuse = -1;
            GLint uniform_MaterialSpecular = -1;
            GLint uniform_MaterialShininess = -1;
            GLint uniform_EdgeAlphaValue = -1;
            GLint uniform_FaceAlphaValue = -1;
            GLint uniform_EdgeThickness = -1;
            GLint uniform_AmbientLightIntensity = -1;
            GLint uniform_NumLights = -1;
            GLint uniform_LightPosition[constants::graphics::lights::NUM];
            GLint uniform_LightColor[constants::graphics::lights::NUM];
            GLint uniform_LightPower[constants::graphics::lights::NUM];
    };
}
