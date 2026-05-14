#include "scene/ObjectFactory.hpp"

#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "scene/Mesh.hpp"
#include "scene/Node.hpp"
#include "rendering/engine/GpuMeshFactory.hpp"

namespace snd3D {
    ObjectFactory::ObjectFactory() {
        this->sphere = std::shared_ptr<GpuMesh>(GpuMeshFactory::createSphere());
        this->cube = std::shared_ptr<GpuMesh>(GpuMeshFactory::createCube());
    }

    Object* ObjectFactory::getFromFile(std::string filePath) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Error loading file: " << importer.GetErrorString() << std::endl;
            throw std::runtime_error("Assimp Error: " + std::string(importer.GetErrorString()));
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
            auto mesh = std::shared_ptr<GpuMesh>(GpuMeshFactory::loadAssimpMesh(scene->mMeshes[i], glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a)));
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

    Object* ObjectFactory::getHits(const EventData* event, const std::unique_ptr<ColorPalette>& colorGetter) {
        if (event == nullptr) return nullptr;

        Node* hits = new Node("Detectors");

        for (const auto& d : event->getDetectors()) {
            Node* detector = new Node(d->getName());
            hits->addChild(detector);
            for (const auto& h : d->getHits()) {
                // Create a mesh with a cube, translate and scale it according to HitData
                Mesh* mesh = new Mesh("Cube", this->cube);
                mesh->setMaterial(std::make_shared<Material>(colorGetter->getColor(h.get())));
                glm::mat4 matrix = translate(glm::mat4(1.0f), h->position);
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
}
