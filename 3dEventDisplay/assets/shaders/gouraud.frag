#version 330 core

// SHADER INPUT (from the vertex shader)
in vec4 ourColor;

// SHADER OUTPUT
out vec4 FragColor;

void main() {
    FragColor = ourColor;
}
