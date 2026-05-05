#include "scene/ObjectFactory.hpp"

#include <vector>
#include <stdexcept>
#include <iostream>

#include <glm/gtc/constants.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "scene/Mesh.hpp"
#include "scene/Node.hpp"

using namespace std;
using namespace glm;

namespace snd3D {
    ObjectFactory::ObjectFactory() {
        this->sphere = shared_ptr<Mesh>(ObjectFactory::createSphere());
        this->cube = shared_ptr<Mesh>(ObjectFactory::createCube());
    }

    Object* ObjectFactory::getFromFile(string filePath) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cerr << "Error loading file: " << importer.GetErrorString() << endl;
            throw runtime_error("Assimp Error: " + string(importer.GetErrorString()));
        }

        return new Object(scene);
    }

    Object* ObjectFactory::getSphere() {
        vector<shared_ptr<Mesh>> meshes;
        meshes.push_back(this->sphere);
        Node* node = new Node("Sphere", meshes);
        return new Object(node);
    }

    Object* ObjectFactory::getCube() {
        vector<shared_ptr<Mesh>> meshes;
        meshes.push_back(this->cube);
        Node* node = new Node("Cube", meshes);
        return new Object(node);
    } 

    Mesh* ObjectFactory::createSphere(vec4 baseColor) {
        vector<vec3> vertices;
        vector<vec4> colors;
        vector<GLuint> indices;
        vector<vec3> normals;

        vec3 center = vec3(0.0, 0.0, 0.0);
        vec3 radius = vec3(1.0f, 1.0f, 1.0f);

        int stacks = 10;  // Number of subdivisions along the Y axis
        int slices = 10;  // Number of subdivisions along the X axis

        // Calc The Vertices
        for (int i = 0; i <= stacks; ++i) {
            float V = i / (float)stacks;
            float phi = V * glm::pi<float>();

            for (int j = 0; j <= slices; ++j) {
                float U = j / (float)slices;
                float theta = U * (2.0f * glm::pi<float>());

                // Spherical coordinates
                float x = center.x + radius.x * (cosf(theta) * sinf(phi));
                float y = center.y + radius.y * cosf(phi);
                float z = center.z + radius.z * (sinf(theta) * sinf(phi));

                vertices.push_back(vec3(x, y, z));
                colors.push_back(baseColor);

                // Normal (for unit sphere normal == position) normalized for safety
                normals.push_back(normalize(vec3(x, y, z)));
            }
        }

        // Calc The Index Positions (two triangles per quad)
        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                int first = (i * (slices + 1)) + j;
                int second = first + slices + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        return new Mesh("SphereGeometry", vertices, colors, normals, indices, baseColor);
    }

    Mesh* ObjectFactory::createCube(vec4 baseColor) {
        vector<vec3> vertices;
        vector<vec4> colors;
        vector<GLuint> indices;
        vector<vec3> normals;

        vertices.push_back(vec3(-1.0, -1.0, 1.0));
        vertices.push_back(vec3(1.0, -1.0, 1.0));
        vertices.push_back(vec3(1.0, 1.0, 1.0));
        vertices.push_back(vec3(-1.0, 1.0, 1.0));

        vertices.push_back(vec3(-1.0, -1.0, -1.0));
        vertices.push_back(vec3(1.0, -1.0, -1.0));
        vertices.push_back(vec3(1.0, 1.0, -1.0));
        vertices.push_back(vec3(-1.0, 1.0, -1.0));

        for (int i = 0; i < vertices.size(); i++) colors.push_back(baseColor);

        indices.push_back(0); indices.push_back(1); indices.push_back(2);
        indices.push_back(2); indices.push_back(3); indices.push_back(0);
        indices.push_back(1); indices.push_back(5); indices.push_back(6);
        indices.push_back(6); indices.push_back(2); indices.push_back(1);
        indices.push_back(7); indices.push_back(6); indices.push_back(5);
        indices.push_back(5); indices.push_back(4); indices.push_back(7);
        indices.push_back(4); indices.push_back(0); indices.push_back(3);
        indices.push_back(3); indices.push_back(7); indices.push_back(4);
        indices.push_back(4); indices.push_back(5); indices.push_back(1);
        indices.push_back(1); indices.push_back(0); indices.push_back(4);
        indices.push_back(3); indices.push_back(2); indices.push_back(6);
        indices.push_back(6); indices.push_back(7); indices.push_back(3);

        normals.push_back(normalize(vec3(0, 0, 1)));
        normals.push_back(normalize(vec3(0, 0, 1)));
        normals.push_back(normalize(vec3(0, 0, 1)));
        normals.push_back(normalize(vec3(0, 0, 1)));
        normals.push_back(normalize(vec3(0, 0, -1)));
        normals.push_back(normalize(vec3(0, 0, -1)));
        normals.push_back(normalize(vec3(0, 0, -1)));
        normals.push_back(normalize(vec3(0, 0, -1)));

        return new Mesh("CubeGeometry", vertices, colors, normals, indices, baseColor);
    }
}
