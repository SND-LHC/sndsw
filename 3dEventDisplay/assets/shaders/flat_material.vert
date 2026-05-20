#version 330 core

layout (location = 0) in vec3 aPos;   // Vertex position from VAO layer 0
layout (location = 1) in vec4 aColor; // Vertex color from VAO layer 1

out vec4 vertexColor;       // Output color processed by this shader

struct Material {
    vec3 baseColor;
};

// Uniform variables set by the main program; they remain the same for every fragment during a draw call
uniform mat4 Projection;    // Projection matrix
uniform mat4 Model;         // Model matrix
uniform mat4 View;          // View matrix
uniform Material material;

void main() {
    /* Transform vertices from object-local coordinates (aPos) to world coordinates
     * (pre-multiplication by Model), then project them into the canonical cube
     * centered at the origin with side length 2 (x, y, z in [-1, 1])
     * (pre-multiplication by the Projection matrix).
     */
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
    vertexColor = vec4(material.baseColor, 1.0); // Set the color as read from the VBO
}

