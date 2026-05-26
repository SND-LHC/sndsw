#include "rendering/engine/GpuMesh.hpp"

#include <stdexcept>

#include "core/Constants.hpp"

using namespace std;

namespace snd3D {

    GpuMesh::GpuMesh(vector<glm::vec3>& vertices, vector<glm::vec4>& colors, vector<glm::vec3>& normals, vector<GLuint>& indices, glm::vec3 anchorPosition) {

        if (vertices.size() == 0) throw runtime_error("Can't create VAO without vertices.");
        if (colors.size() != vertices.size()) throw runtime_error("The size of colors must match the size of vertices.");
        if (normals.size() != vertices.size()) throw runtime_error("The size of normals must match the size of vertices.");

        this->numIndices = indices.size();
        this->numVertices = vertices.size();
        this->anchor = anchorPosition;

        // Add the anchor to the vectors so it's copied into the VBOs
        vertices.push_back(anchorPosition);
        colors.push_back(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
        normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // Dummy normal
        indices.push_back(this->numVertices);

        // Generate the VAO and store the location
        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);

        // Generates, activates and fills the vertices VBO
        glGenBuffers(1, &this->vboVertices);
        glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

        // Loading the vertices VBO into layer 0
        glVertexAttribPointer(constants::graphics::VERTICES_LAYER, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(constants::graphics::VERTICES_LAYER);

        // Generates, activates and fills the colors VBO
        glGenBuffers(1, &this->vboColors);
        glBindBuffer(GL_ARRAY_BUFFER, this->vboColors);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);

        // Loading the colors VBO into layer 1
        glVertexAttribPointer(constants::graphics::COLORS_LAYER, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(constants::graphics::COLORS_LAYER);

        // Generates, activates and fills the normals VBO
        glGenBuffers(1, &this->vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, this->vboNormals);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

        // Loading the normals VBO into layer 2
        glVertexAttribPointer(constants::graphics::NORMALS_LAYER, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(constants::graphics::NORMALS_LAYER);

        // Generates, activates and fills the EBO
        glGenBuffers(1, &this->eboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->eboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // VAO, VBO and EBO unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Restore the geometry after the anchor is copied
        vertices.pop_back();
        colors.pop_back();
        normals.pop_back();
        indices.pop_back();
    }

    GpuMesh::~GpuMesh() {
        // Free GPU resources when the mesh is destroyed
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDeleteVertexArrays(1, &this->vao);
        glDeleteBuffers(1, &this->vboVertices);
        glDeleteBuffers(1, &this->vboColors);
        glDeleteBuffers(1, &this->vboNormals);
        glDeleteBuffers(1, &this->eboIndices);
    }

    void GpuMesh::setRenderMode(GLenum _renderMode) {
        this->renderMode = _renderMode;
    }

    void GpuMesh::render(bool showAnchor) {
        // RENDER!
        glBindVertexArray(this->vao);
        glDrawElements(this->renderMode, this->numIndices, GL_UNSIGNED_INT, 0);

        if (showAnchor) {
            glPointSize(constants::sizes::ANCHOR);
            glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, BUFFER_OFFSET(this->numIndices * sizeof(GLuint)));
        }
    }

    const glm::vec3& GpuMesh::getAnchor() {
        return this->anchor;
    }
}
