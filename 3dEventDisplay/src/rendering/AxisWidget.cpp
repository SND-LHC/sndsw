/**
 * @file AxisWidget.cpp
 * @brief Implementation of the 3D axis widget for orientation visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/AxisWidget.hpp"

#include <vector>

#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>

#include "core/Constants.hpp"

using namespace std;
using namespace glm;

namespace snd3D {

    AxisWidget::AxisWidget(shared_ptr<Shader> _shader, const int _size, const int _margin) {
        this->shader = _shader;
        this->projectionMatrix = ortho(-1.5f, 1.5f, -1.5f, 1.5f, -2.0f, 2.0f); // Use always an orthographic projection
        this->totalSize = _size + _margin;
        this->margin = _margin;

        vector<vec3> vertices;
        vector<vec4> colors;

        // X Axis: Red
        vertices.push_back(vec3(0.0f, 0.0f, 0.0f));
        vertices.push_back(vec3(1.0f, 0.0f, 0.0f));
        colors.push_back(vec4(1.0f, 0.0f, 0.0f, 1.0f));
        colors.push_back(vec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Y Axis: Green
        vertices.push_back(vec3(0.0f, 0.0f, 0.0f));
        vertices.push_back(vec3(0.0f, 1.0f, 0.0f));
        colors.push_back(vec4(0.0f, 1.0f, 0.0f, 1.0f));
        colors.push_back(vec4(0.0f, 1.0f, 0.0f, 1.0f));

        // Z Axis: Blue
        vertices.push_back(vec3(0.0f, 0.0f, 0.0f));
        vertices.push_back(vec3(0.0f, 0.0f, 1.0f));
        colors.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
        colors.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));

        // BUFFER CREATION
        glGenVertexArrays(1, &vao);
        glBindVertexArray(this->vao);

        // Generates, activates and fills the vertices VBO
        glGenBuffers(1, &this->vboVertices);
        glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);

        // Loading the vertices VBO into layer 0
        glVertexAttribPointer(constants::graphics::VERTICES_LAYER, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(constants::graphics::VERTICES_LAYER);

        // Generates, activates and fills the colors VBO
        glGenBuffers(1, &this->vboColors);
        glBindBuffer(GL_ARRAY_BUFFER, this->vboColors);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_STATIC_DRAW);

        // Loading the colors VBO into layer 1
        glVertexAttribPointer(constants::graphics::COLORS_LAYER, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(constants::graphics::COLORS_LAYER);

        // VAO and VBO unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    AxisWidget::~AxisWidget() {
        // Free GPU resources when the axes are destroyed
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDeleteVertexArrays(1, &this->vao);
        glDeleteBuffers(1, &this->vboVertices);
        glDeleteBuffers(1, &this->vboColors);
    }

    void AxisWidget::draw(const mat4& viewMatrix, int screenWidth, int screenHeight) {

        mat4 viewRotation = mat4(mat3(viewMatrix)); // Keep only the rotation component

        // Set the axes in the bottom right side
        glViewport(screenWidth - this->totalSize - this->margin, this->margin, this->totalSize, this->totalSize);

        // Save the old state and disable the depth test to have the axes always in the foreground
        GLint isDepthTestEnabled = 0;
        glGetIntegerv(GL_DEPTH_TEST, &isDepthTestEnabled);
        glDisable(GL_DEPTH_TEST);

        // Set line width for the axes
        float prevLineWidth;
        glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
        glLineWidth(constants::sizes::AXIS);

        // Enable the shader and set the uniforms
        shader->use();
        shader->bindGlobalUniforms(viewRotation, this->projectionMatrix);
        shader->bindLocalUniforms(mat4(1.0f), NULL);

        // Draw the axes
        glBindVertexArray(this->vao);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);

        this->printLabel(vec3(1, 0, 0), "X", IM_COL32(255, 0, 0, 255), viewRotation, screenWidth, screenHeight);
        this->printLabel(vec3(0, 1, 0), "Y", IM_COL32(0, 255, 0, 255), viewRotation, screenWidth, screenHeight);
        this->printLabel(vec3(0, 0, 1), "Z", IM_COL32(0, 0, 255, 255), viewRotation, screenWidth, screenHeight);

        // Restore old OpenGL states
        glLineWidth(prevLineWidth);
        if (isDepthTestEnabled) glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, screenWidth, screenHeight);
    }

    void AxisWidget::printLabel(vec3 position, const char* label, ImU32 color, const mat4& viewRotation, int screenWidth, int screenHeight) {
        // Transform world coordinates to clip space: multiply by 1.1 so the label is a little further away than the end of the axis
        vec4 clipPos = this->projectionMatrix * viewRotation * vec4(position * 1.1f, 1.0f);

        // Map clip space to Local Viewport Coordinates
        vec2 localPos = (vec2(clipPos) / clipPos.w + 1.0f) * 0.5f * (float)this->totalSize;

        // Final screen positioning
        float screenX = screenWidth - this->totalSize - this->margin + localPos.x;
        float screenY = screenHeight - (this->margin + localPos.y);

        // Render
        ImGui::GetForegroundDrawList()->AddText(ImVec2(screenX, screenY), color, label);
    }
}
