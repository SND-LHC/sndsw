#version 330 core

#define MAX_LIGHTS 4 // Keep the value updated with the cpp code and other shaders

// Structure representing a point light
struct PointLight {
	vec3 position;
	vec3 color;
	float power;
};

// Per-vertex data taken from VBO
layout (location = 0) in vec3 aPos;     // Vertex position
layout (location = 1) in vec4 aColor;   // Vertex color
layout (location = 2) in vec3 vertexNormal; // Normal vectors

// UNIFORM VARIABLES
uniform mat4 Projection;
uniform mat4 Model;
uniform mat4 View;
uniform PointLight lights[MAX_LIGHTS]; // Propreties of each light in the scene
uniform int numLights;

// SHADER OUTPUTS TO THE FRAGMENT
out vec3 n, v, l[MAX_LIGHTS];

void main() {
    // Transform vertices from object-local coordinates (aPos) to world/view/clip space.
    // First multiply by Model to move to world space, then by View to move to camera (eye) space,
    // and finally by Projection to project into normalized device coordinates (cube centered at origin with x,y,z in [-1, 1]).
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);

    // Transform the vertex coordinates (aPos) into view space
    vec4 eyePosition = View * Model * vec4(aPos, 1.0);

    // Transform the vertex normal into view space
    n = normalize(transpose(inverse(mat3(View * Model))) * vertexNormal);

    // Compute view direction V
    v = normalize(-eyePosition.xyz);

    // Compute vectors that the fragment will use the contribution for each light
    for (int i = 0; i < numLights; i++) {

        // Transform light position in VCS
        vec4 eyeLightPos = View * vec4(lights[i].position, 1.0);

        // Compute light direction L
        l[i] = normalize((eyeLightPos - eyePosition).xyz);
    }
}
