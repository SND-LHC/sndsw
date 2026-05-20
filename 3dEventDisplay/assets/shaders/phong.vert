#version 330 core

#define NUM_LIGHTS 2 // Keep the value updated with the cpp code and other shaders

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
uniform vec3 ViewPos;
uniform PointLight lights[NUM_LIGHTS]; // Propreties of each light in the scene

// SHADER OUTPUTS TO THE FRAGMENT
out vec3 N, L[NUM_LIGHTS], R[NUM_LIGHTS], V[NUM_LIGHTS];

void main() {
    // Transform vertices from object-local coordinates (aPos) to world/view/clip space.
    // First multiply by Model to move to world space, then by View to move to camera (eye) space,
    // and finally by Projection to project into normalized device coordinates (cube centered at origin with x,y,z in [-1, 1]).
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);

    // Transform the vertex coordinates (aPos) into view space
    vec4 eyePosition = View * Model * vec4(aPos, 1.0);

    // Transform the vertex normal into view space
    N = normalize(transpose(inverse(mat3(View * Model))) * vertexNormal);

    // Compute vectors that the fragment will use the contribution for each light
    for (int i = 0; i < NUM_LIGHTS; i++) {

        // Transform light position in VCS
        vec4 eyeLightPos = View * vec4(lights[i].position, 1.0);

        // Compute light direction L, reflection direction R and view direction V
        V[i] = normalize(ViewPos - eyePosition.xyz);
        L[i] = normalize((eyeLightPos - eyePosition).xyz);
        R[i] = normalize(reflect(-L[i], N));  // Construct the reflection direction of L with respect to the normal
    }
}
