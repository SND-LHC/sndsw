#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec4 vColor;
out vec3 vLocalPos; // Keep also the local position

void main() {
    vLocalPos = aPos;
    vColor = aColor;
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}