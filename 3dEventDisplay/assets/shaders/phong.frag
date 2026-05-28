#version 330 core

#define MAX_LIGHTS 4 // Keep the value updated with the cpp code and other shaders

// Structure representing a point light
struct PointLight {
	vec3 position;
	vec3 color;
	float power;
};

// Structure holding material informations
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// UNIFORM VARIABLES
uniform Material material; // Material propreties
uniform PointLight lights[MAX_LIGHTS]; // Propreties of each light in the scene
uniform int numLights;
uniform float uAmbientLightIntensity;

// SHADER INPUT (from the vertex shader)
in vec3 N, V, L[MAX_LIGHTS];

// SHADER OUTPUTS
out vec4 FragColor;

void main() {

    // ----- AMBIENT COMPONENT -----
    vec3 ambient = uAmbientLightIntensity * material.ambient;

    // Light contributions accumulator, starting with ambient light (which is not depending on the lights)
    vec3 baseColor = ambient;

    // Compute contribution for each light
    for (int i = 0; i < numLights; i++) {

        // ----- DIFFUSE COMPONENT -----
        float cos_theta = max(dot(L[i], N), 0);
        vec3 diffuse = lights[i].color * cos_theta * material.diffuse;

        // ----- SPECULAR COMPONENT -----
        vec3 H = normalize(L[i] + V);
        float cos_alfa = pow(max(dot(H, N), 0), material.shininess * 4);
        vec3 specular = lights[i].color * cos_alfa * material.specular;

        // Accumulate the contribution given by this light
        baseColor += lights[i].power * (diffuse + specular);
    }

    FragColor = vec4(baseColor, 1);
}
