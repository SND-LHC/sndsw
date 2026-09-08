#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vLocalPos[];
in vec4 vColor[];

out vec3 localPos;
out vec3 barycentric;
out vec4 color;

void main() {
    for(int i = 0; i < 3; i++) {
        // Assign different barycentric coordinates for each extreme vertex
        barycentric = vec3(0.0);
        barycentric[i] = 1.0;

        // Keep these values unchanged
        localPos = vLocalPos[i];
        color = vColor[i];
        gl_Position = gl_in[i].gl_Position;

        EmitVertex();
    }
    EndPrimitive();
}