#version 330 core

// NB: THE VALUE CAN MAKE THE SHADER LINKING FAIL: IF SO, LOWER IT
#define MAX_LIGHTS 5 // Keep the value updated with the cpp code and other shaders

// Structure representing a point light
struct PointLight {
	vec3 position;
	vec3 color;
	float power;
};

// Structure holding material informations
struct Material {
    vec3 baseColor;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// UNIFORM VARIABLES
uniform int numLights;  // Number of lights really used
uniform Material material; // Material propreties
uniform bool uUseBlinnPhong; // True if BlinnPhong model is used, false if Phong
uniform PointLight lights[MAX_LIGHTS]; // Propreties of each light in the scene

float ambientLightIntensity = 0.2;

// SHADER INPUT (from the vertex shader)
in vec3 N, L[MAX_LIGHTS], R[MAX_LIGHTS], V[MAX_LIGHTS]; // Interpolated vectors for each light
in vec2 vTexCoord;

// SHADER OUTPUTS
out vec4 FragColor;

void main() {

    // ----- AMBIENT COMPONENT -----
    vec3 ambient = ambientLightIntensity * material.ambient;

    // Light contributions accumulator, starting with ambient light (which is not depending on the lights)
    vec3 baseColor = ambient;

    // Compute contribution for each light
    for (int i = 0; i < numLights; i++) {

        // ----- DIFFUSE COMPONENT -----
        float cos_theta = max(dot(L[i], N), 0);
        vec3 diffuse = lights[i].color * cos_theta * material.diffuse;

        // ----- SPECULAR COMPONENT -----
        float cos_alfa;
        // Changes the calculation of specular reflection of light based on the chosen lighting model
        if (uUseBlinnPhong) {
            vec3 H = normalize(L[i] + V[i]);
            cos_alfa = pow(max(dot(H, N), 0), material.shininess * 4);
        } else {
            cos_alfa = pow(max(dot(V[i], R[i]), 0), material.shininess);
        }
        vec3 specular = lights[i].color * cos_alfa * material.specular;

        // Accumulate the contribution given by this light
        baseColor += lights[i].power * (diffuse + specular);
    }

    FragColor = vec4(baseColor, 1);
}
