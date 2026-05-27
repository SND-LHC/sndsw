#version 330 core

#define MAX_LIGHTS 4 // Keep the value updated with the cpp code and other shaders

struct IlluminationData {
    vec3 N;
    vec3 V;
    vec3 L[MAX_LIGHTS];
};

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform int numLights;

in IlluminationData vIlluminationData[];
in vec3 vLocalPos[];

out vec3 localPos;
out vec3 barycentric;
out IlluminationData illuminationData;

void main() {
    for(int i = 0; i < 3; i++) {
        // Assign different barycentric coordinates for each extreme vertex
        barycentric = vec3(0.0);
        barycentric[i] = 1.0;

        // Keep these values unchanged
        localPos = vLocalPos[i];
        gl_Position = gl_in[i].gl_Position;
        
        illuminationData.N = vIlluminationData[i].N;
        illuminationData.V = vIlluminationData[i].V;

        for (int l = 0; l < numLights; l++) {
            illuminationData.L[l] = vIlluminationData[i].L[l];
        }

        EmitVertex();
    }
    EndPrimitive();
}
