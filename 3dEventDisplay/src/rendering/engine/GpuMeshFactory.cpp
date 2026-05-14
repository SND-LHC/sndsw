#include "rendering/engine/GpuMeshFactory.hpp"

#include <vector>
#include <iostream>

#include <glm/gtc/constants.hpp>

using namespace std;
using namespace glm;

namespace snd3D {
    GpuMesh* GpuMeshFactory::createSphere(vec4 baseColor) {
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

    GpuMesh* GpuMeshFactory::createCube(vec4 baseColor) {
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

    GpuMesh* GpuMeshFactory::loadAssimpMesh(aiMesh* mesh, vec4 baseColor) {
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
