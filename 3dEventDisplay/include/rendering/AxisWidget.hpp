/**
 * @file AxisWidget.hpp
 * @brief Renders a 3D coordinate axis widget for visual orientation reference.
 *
 * AxisWidget displays a small interactive 3D coordinate system (X, Y, Z axes with
 * corresponding colors: red, green, blue) positioned in the corner of the viewport.
 * Uses orthographic projection and dynamically computes label positions to maintain
 * readability from any camera angle. Helps users maintain spatial awareness during
 * scene navigation and rotation.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <memory>

#include <imgui.h>
#include <glm/glm.hpp>

#include "rendering/engine/Shader.hpp"

namespace snd3D {
    class AxisWidget {
        public:
            AxisWidget(std::shared_ptr<Shader> _shader, const int _size, const int _margin);
            ~AxisWidget();

            void draw(const glm::mat4& viewMatrix, int screenWidth, int screenHeight);

        private:
            // GPU buffers location
            GLuint vao = 0;     // Vertex Array Object (VAO) identifier
            GLuint vboVertices = 0; // Vertex Buffer Object (VBO) identifier containing the object's vertex coordinates
            GLuint vboColors = 0;   // VBO identifier containing the object's vertex colors

            std::shared_ptr<Shader> shader;
            glm::mat4 projectionMatrix;

            int totalSize;
            int margin;

            void printLabel(glm::vec3 pos, const char* label, ImU32 color, const glm::mat4& viewRotation, int screenWidth, int screenHeight);
    };
}