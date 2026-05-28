#include "rendering/engine/Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "rendering/engine/ShaderMaker.hpp"

namespace snd3D {

    Shader::Shader(std::string _name, std::string _vertexShader, std::string _fragmentShader, std::string _geometryShader) {
        this->programId = ShaderMaker::createProgram(
            std::string(constants::paths::SHADER) + _vertexShader,
            std::string(constants::paths::SHADER) + _fragmentShader,
            _geometryShader.empty() ? "" : std::string(constants::paths::SHADER) + _geometryShader
        );
        this->name = std::string(_name);

        this->uniform_Model = glGetUniformLocation(this->programId, "Model");
        this->uniform_Projection = glGetUniformLocation(this->programId, "Projection");
        this->uniform_View = glGetUniformLocation(this->programId, "View");
        this->uniform_ViewPos = glGetUniformLocation(this->programId, "ViewPos");
        this->uniform_MaterialBaseColor = glGetUniformLocation(this->programId, "material.baseColor");
        this->uniform_MaterialAmbient = glGetUniformLocation(this->programId, "material.ambient");
        this->uniform_MaterialDiffuse = glGetUniformLocation(this->programId, "material.diffuse");
        this->uniform_MaterialSpecular = glGetUniformLocation(this->programId, "material.specular");
        this->uniform_MaterialShininess = glGetUniformLocation(this->programId, "material.shininess");
        this->uniform_EdgeAlphaValue = glGetUniformLocation(this->programId, "uEdgeAlpha");
        this->uniform_FaceAlphaValue = glGetUniformLocation(this->programId, "uFaceAlpha");
        this->uniform_EdgeThickness = glGetUniformLocation(this->programId, "uEdgeThickness");
        this->uniform_AmbientLightIntensity = glGetUniformLocation(this->programId, "uAmbientLightIntensity");
        this->uniform_NumLights = glGetUniformLocation(this->programId, "numLights");
        for (int i = 0; i < constants::graphics::lights::NUM; i++) {
            std::string base = "lights[" + std::to_string(i) + "].";
            this->uniform_LightPosition[i] = glGetUniformLocation(this->programId, (base + "position").c_str());
            this->uniform_LightColor[i] = glGetUniformLocation(this->programId, (base + "color").c_str());
            this->uniform_LightPower[i] = glGetUniformLocation(this->programId, (base + "power").c_str());
        }
    }

    Shader::~Shader() {
        glDeleteProgram(this->programId);
    }

    std::string Shader::getName() {
        return name;
    }

    GLuint Shader::getProgramId() {
        return programId;
    }

    void Shader::use() {
        glUseProgram(this->programId);
    }

    void Shader::bindGlobalUniforms(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& camPos, const float ambientLight, const std::vector<std::unique_ptr<PointLight>>& lights, const float edgeAlphaValue, const float faceAlphaValue, const float edgeThickness) {
        if (this->uniform_Projection != -1) glUniformMatrix4fv(this->uniform_Projection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        if (this->uniform_View != -1) glUniformMatrix4fv(this->uniform_View, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        if (this->uniform_ViewPos != -1) glUniform3f(this->uniform_ViewPos, camPos.x, camPos.y, camPos.z);
        if (this->uniform_EdgeAlphaValue != -1) glUniform1f(this->uniform_EdgeAlphaValue, edgeAlphaValue);
        if (this->uniform_FaceAlphaValue != -1) glUniform1f(this->uniform_FaceAlphaValue, faceAlphaValue);
        if (this->uniform_EdgeThickness != -1) glUniform1f(this->uniform_EdgeThickness, edgeThickness);
        if (this->uniform_AmbientLightIntensity != -1) glUniform1f(this->uniform_AmbientLightIntensity, ambientLight);
        if (this->uniform_NumLights != -1) glUniform1i(this->uniform_NumLights, (int)lights.size());
        for (int i = 0; i < std::min((int)lights.size(), constants::graphics::lights::NUM); i++) {
            if (this->uniform_LightColor[i] != -1) glUniform3f(this->uniform_LightColor[i], lights.at(i)->getColor().x, lights.at(i)->getColor().y, lights.at(i)->getColor().z);
            if (this->uniform_LightPosition[i] != -1) glUniform3f(this->uniform_LightPosition[i], lights.at(i)->getPosition().x, lights.at(i)->getPosition().y, lights.at(i)->getPosition().z);
            if (this->uniform_LightPower[i] != -1) glUniform1f(this->uniform_LightPower[i], lights.at(i)->getPower());
        }
    }

    void Shader::bindLocalUniforms(const glm::mat4& modelMatrix, Material* material) {
        // Check for uniform existence in the active shader; if present, update them
        if (this->uniform_Model != -1) glUniformMatrix4fv(this->uniform_Model, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        if (this->uniform_MaterialBaseColor != -1 && material != nullptr) glUniform3fv(this->uniform_MaterialBaseColor, 1, glm::value_ptr(material->getBaseColor()));
        if (this->uniform_MaterialAmbient != -1 && material != nullptr) glUniform3fv(this->uniform_MaterialAmbient, 1, glm::value_ptr(material->getAmbient()));
        if (this->uniform_MaterialDiffuse != -1 && material != nullptr) glUniform3fv(this->uniform_MaterialDiffuse, 1, glm::value_ptr(material->getDiffuse()));
        if (this->uniform_MaterialSpecular != -1 && material != nullptr) glUniform3fv(this->uniform_MaterialSpecular, 1, glm::value_ptr(material->getSpecular()));
        if (this->uniform_MaterialShininess != -1 && material != nullptr) glUniform1f(this->uniform_MaterialShininess, material->getShininess());
    }
}
