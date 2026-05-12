#include "scene/ObjectFactory.hpp"

#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "scene/Mesh.hpp"
#include "scene/Node.hpp"
#include "rendering/engine/Material.hpp"

using namespace std;
using namespace glm;

namespace snd3D {
    ObjectFactory::ObjectFactory() {
        this->sphere = shared_ptr<GpuMesh>(ObjectFactory::createSphere());
        this->cube = shared_ptr<GpuMesh>(ObjectFactory::createCube());
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

        // Load all materials
        std::vector<std::shared_ptr<Material>> materials;
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            materials.push_back(std::make_shared<Material>(scene->mMaterials[i]));
        }

        // Load all meshes
        std::vector<std::shared_ptr<GpuMesh>> meshes;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiColor4D baseColor(0.6f, 0.6f, 0.6f, 1.0f); // Default color
            scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->Get(AI_MATKEY_BASE_COLOR, baseColor);
            auto mesh = std::shared_ptr<GpuMesh>(ObjectFactory::loadAssimpMesh(scene->mMeshes[i], glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a)));
            meshes.push_back(mesh);
        }

        return new Object(scene, meshes, materials);
    }

    Object* ObjectFactory::getSphere() {
        Mesh* mesh = new Mesh("SphereMesh", this->sphere);
        Node* node = new Node("Sphere");
        node->addMesh(mesh);
        return new Object(node);
    }

    Object* ObjectFactory::getCube() {
        Mesh* mesh = new Mesh("CubeMesh", this->cube);
        Node* node = new Node("Cube");
        node->addMesh(mesh);
        return new Object(node);
    } 

    Object* ObjectFactory::getHits(const EventData* event) {
        if (event == nullptr) return nullptr;

        Node* hits = new Node("Detectors");

        for (const auto& d : event->getDetectors()) {
            Node* detector = new Node(d->getName());
            hits->addChild(detector);
            for (const auto& h : d->getHits()) {
                // Create a mesh with a cube, translate and scale it according to HitData
                Mesh* mesh = new Mesh("Cube", this->cube);
                mat4 matrix = translate(mat4(1.0f), h->position);
                matrix = scale(matrix, h->radius);

                std::stringstream ss;
                ss << std::fixed << std::setprecision(1);
                ss << "(" << h->position.x << ", " << h->position.y << ", " << h->position.z << ")";

                Node* hit = new Node(ss.str(), matrix);
                hit->addMesh(mesh);
                detector->addChild(hit);
            }
        }

        return new Object(hits);
    } 

    GpuMesh* ObjectFactory::createSphere(vec4 baseColor) {
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

        return new GpuMesh(vertices, colors, normals, indices, vertices[0]);
    }

    GpuMesh* ObjectFactory::createCube(vec4 baseColor) {
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

        for (int i = 0; i < (int)vertices.size(); i++) colors.push_back(baseColor);

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

        return new GpuMesh(vertices, colors, normals, indices, vertices[0]);
    }

    GpuMesh* ObjectFactory::loadAssimpMesh(aiMesh* mesh, vec4 baseColor) {
        vector<vec3> vertices;
        vector<vec4> colors;
        vector<GLuint> indices;
        vector<vec3> normals;

        // Fill vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D pos = mesh->mVertices[i];
            vertices.push_back(vec3(pos.x, pos.y, pos.z));
        }

        // Fill Colors
        if (mesh->HasVertexColors(0)) {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiColor4D c = mesh->mColors[0][i];
                colors.push_back(vec4(c.r, c.g, c.b, c.a));
            }
        } else {
            // cerr << "Colors data missing - Using vec4(0.6f, 0.6f, 0.6f, 1.0f)" << endl;
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                colors.push_back(baseColor);
            }
        }

        // Fill vertices normal
        if (mesh->HasNormals()) {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D n = mesh->mNormals[i];
                normals.push_back(vec3(n.x, n.y, n.z));
            }
        } else {
            cerr << "Normals data missing - Using vec3(0)" << endl;
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                normals.push_back(vec3(0));
            }
        }

        // Fill face indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            // Assume the model has only triangles, so 3 indices per vertex
            indices.push_back(mesh->mFaces[i].mIndices[0]);
            indices.push_back(mesh->mFaces[i].mIndices[1]);
            indices.push_back(mesh->mFaces[i].mIndices[2]);
        }

        return new GpuMesh(vertices, colors, normals, indices, vertices[0]);
    }
}
