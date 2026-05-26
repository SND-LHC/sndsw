#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace snd3D {
    class GpuMesh {
        public:
            GpuMesh(
                std::vector<glm::vec3>& vertices,
                std::vector<glm::vec4>& colors,
                std::vector<glm::vec3>& normals,
                std::vector<GLuint>& indices,
                glm::vec3 anchorPosition
            );
            ~GpuMesh();
            void setRenderMode(GLenum _renderMode);
            void render(bool showAnchor);
            const glm::vec3& getAnchor();

        private:
            unsigned int numIndices, numVertices; // The number of vertices and indices without considering the anchor

            // GPU buffers location
            GLuint vao = 0;     // Vertex Array Object (VAO) identifier
            GLuint vboVertices = 0; // Vertex Buffer Object (VBO) identifier containing the object's vertex coordinates
            GLuint vboColors = 0;   // VBO identifier containing the object's vertex colors
            GLuint vboNormals = 0;  // VBO identifier containing the vertex normals
            GLuint eboIndices = 0;  // Element Buffer Object (EBO) identifier

            GLenum renderMode = GL_TRIANGLES; // Vertex interpretation mode for the rendering stage (ex. GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_LINES, GL_POINTS)

            glm::vec3 anchor;
    };
}
