#version 330 core

#define MAX_LIGHTS 4 // Keep the value updated with the cpp code and other shaders
#define COLOR_AMPLIFIER 6

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

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// UNIFORM VARIABLES
uniform float uFaceAlpha;
uniform float uEdgeAlpha;
uniform float uEdgeThickness;
uniform Material material; // Material propreties
uniform PointLight lights[MAX_LIGHTS]; // Propreties of each light in the scene
uniform int numLights;

float ambientLightIntensity = 0.4;

in vec3 localPos;
in vec3 barycentric; // The interpolation will fill the correct barycentric coordinates for each fragment
in IlluminationData illuminationData;

out vec4 FragColor;

void main() {
    /*
     * COLOR COMPUTATION
     */

    // ----- AMBIENT COMPONENT -----
    vec3 ambient = ambientLightIntensity * material.ambient;

    // Light contributions accumulator, starting with ambient light (which is not depending on the lights)
    vec3 baseColor = ambient;

    // Compute contribution for each light
    for (int i = 0; i < numLights; i++) {

        // ----- DIFFUSE COMPONENT -----
        float cos_theta = max(dot(illuminationData.L[i], illuminationData.N), 0);
        vec3 diffuse = lights[i].color * cos_theta * material.diffuse;

        // ----- SPECULAR COMPONENT -----
        vec3 H = normalize(illuminationData.L[i] + illuminationData.V);
        float cos_alfa = pow(max(dot(H, illuminationData.N), 0), material.shininess * 4);
        vec3 specular = lights[i].color * cos_alfa * material.specular;

        // Accumulate the contribution given by this light
        baseColor += lights[i].power * (diffuse + specular);
    }


    /*
     * TRANSPARENCY COMPUTATION
     */

    vec3 d = fwidth(barycentric); // Calculate the intensity of change (derivative)
    vec3 a3 = smoothstep(vec3(0.0), d * uEdgeThickness, barycentric); // Create a smooth gradient based on barycentric distance
    float edgeFactor = min(min(a3.x, a3.y), a3.z); // Find the closest edge to determine the wireframe factor

    float finalAlpha = mix(uEdgeAlpha, uFaceAlpha, edgeFactor); // Higher alpha for edges and lower for faces

    FragColor = vec4(vec3(baseColor * COLOR_AMPLIFIER), finalAlpha); // Output the color replacing the computed transparency
}
