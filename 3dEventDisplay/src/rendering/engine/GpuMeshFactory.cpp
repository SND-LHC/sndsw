/**
 * @file GpuMeshFactory.cpp
 * @brief Implementation of GPU mesh factory for creating standard mesh shapes.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/engine/GpuMeshFactory.hpp"

#include <iostream>
#include <cfloat>

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

        return new GpuMesh(vertices, colors, normals, indices, computeCenter(vertices));
    }

    GpuMesh* GpuMeshFactory::createCube(vec4 baseColor) {
        vector<vec3> vertices;
        vector<vec4> colors;
        vector<GLuint> indices;
        vector<vec3> normals;

        // Frontal face
        vertices.push_back(vec3(-1, -1,  1)); normals.push_back(vec3( 0,  0,  1));
        vertices.push_back(vec3( 1, -1,  1)); normals.push_back(vec3( 0,  0,  1));
        vertices.push_back(vec3( 1,  1,  1)); normals.push_back(vec3( 0,  0,  1));
        vertices.push_back(vec3(-1,  1,  1)); normals.push_back(vec3( 0,  0,  1));

        // Rear face
        vertices.push_back(vec3( 1, -1, -1)); normals.push_back(vec3( 0,  0, -1));
        vertices.push_back(vec3(-1, -1, -1)); normals.push_back(vec3( 0,  0, -1));
        vertices.push_back(vec3(-1,  1, -1)); normals.push_back(vec3( 0,  0, -1));
        vertices.push_back(vec3( 1,  1, -1)); normals.push_back(vec3( 0,  0, -1));

        // Right face
        vertices.push_back(vec3( 1, -1,  1)); normals.push_back(vec3( 1,  0,  0));
        vertices.push_back(vec3( 1, -1, -1)); normals.push_back(vec3( 1,  0,  0));
        vertices.push_back(vec3( 1,  1, -1)); normals.push_back(vec3( 1,  0,  0));
        vertices.push_back(vec3( 1,  1,  1)); normals.push_back(vec3( 1,  0,  0));

        // Left face
        vertices.push_back(vec3(-1, -1, -1)); normals.push_back(vec3(-1,  0,  0));
        vertices.push_back(vec3(-1, -1,  1)); normals.push_back(vec3(-1,  0,  0));
        vertices.push_back(vec3(-1,  1,  1)); normals.push_back(vec3(-1,  0,  0));
        vertices.push_back(vec3(-1,  1, -1)); normals.push_back(vec3(-1,  0,  0));

        // Upper face
        vertices.push_back(vec3(-1,  1,  1)); normals.push_back(vec3( 0,  1,  0));
        vertices.push_back(vec3( 1,  1,  1)); normals.push_back(vec3( 0,  1,  0));
        vertices.push_back(vec3( 1,  1, -1)); normals.push_back(vec3( 0,  1,  0));
        vertices.push_back(vec3(-1,  1, -1)); normals.push_back(vec3( 0,  1,  0));

        // Lower face
        vertices.push_back(vec3(-1, -1, -1)); normals.push_back(vec3( 0, -1,  0));
        vertices.push_back(vec3( 1, -1, -1)); normals.push_back(vec3( 0, -1,  0));
        vertices.push_back(vec3( 1, -1,  1)); normals.push_back(vec3( 0, -1,  0));
        vertices.push_back(vec3(-1, -1,  1)); normals.push_back(vec3( 0, -1,  0));

        // Generate indices for the 6 faces (2 triangles per face) using a 4-vertex offset loop
        for (int f = 0; f < 6; f++) {
            int b = f * 4;
            indices.push_back(b);   indices.push_back(b+1); indices.push_back(b+2);
            indices.push_back(b+2); indices.push_back(b+3); indices.push_back(b);
        }

        for (int i = 0; i < (int)vertices.size(); i++) colors.push_back(baseColor);

        return new GpuMesh(vertices, colors, normals, indices, computeCenter(vertices));
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

        return new GpuMesh(vertices, colors, normals, indices, computeCenter(vertices));
    }

    vec3 GpuMeshFactory::computeCenter(const vector<vec3>& vertices) {
        vec3 minV( FLT_MAX,  FLT_MAX,  FLT_MAX);
        vec3 maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& v : vertices) {
            minV = glm::min(minV, v);
            maxV = glm::max(maxV, v);
        }

        return (minV + maxV) * 0.5f;
    }
}
