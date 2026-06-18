#version 330 core
#define MAX_LIGHTS 4 // Keep the value updated with the cpp code and other shaders

// Structure representing a point light
struct PointLight {
	vec3 position;
	vec3 color;
	float power;
};

struct IlluminationData {
    vec3 N;
    vec3 V;
    vec3 L[MAX_LIGHTS];
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 vertexNormal;

// UNIFORM VARIABLES
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform PointLight lights[MAX_LIGHTS]; // Propreties of each light in the scene
uniform int numLights;

// SHADER OUTPUTS TO THE GEOMETRY
out IlluminationData vIlluminationData;
out vec3 vLocalPos;

void main() {
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
    vLocalPos = aPos;

    vec4 eyePosition = View * Model * vec4(aPos, 1.0);

    // Transform the vertex normal into view space
    vIlluminationData.N = normalize(transpose(inverse(mat3(View * Model))) * vertexNormal);

    // Compute view direction V
    vIlluminationData.V = normalize(-eyePosition.xyz);

    // Compute vectors that the fragment will use the contribution for each light
    for (int i = 0; i < numLights; i++) {

        // Transform light position in VCS
        vec4 eyeLightPos = View * vec4(lights[i].position, 1.0);

        // Compute light direction L
        vIlluminationData.L[i] = normalize((eyeLightPos - eyePosition).xyz);
    }
}
