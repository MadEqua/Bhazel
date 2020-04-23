#include "bzpch.h"

#include "Mesh.h"
#include "Renderer.h"
#include "Core/Application.h"
#include "Core/Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


namespace BZ {

    Mesh Mesh::createUnitCube(const Material &material) {
        constexpr uint32 CUBE_VERTEX_COUNT = 36;

        constexpr float NEG_ONE_F = -1.0f;
        constexpr float ONE_F = 1.0f;

        constexpr uint16 ZERO = 0;
        constexpr uint16 ONE_UI = 0xffff - 1;

        Mesh::Vertex cubeVertices[CUBE_VERTEX_COUNT] = {
            //Front
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },

            //Right
            { { ONE_F, NEG_ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ONE_UI } },

            //Left
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ONE_UI } },

            //Back
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },

            //Bottom
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },

            //Top
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
        };

        return Mesh(cubeVertices, CUBE_VERTEX_COUNT, material);
    }

    Mesh Mesh::createUnitCubeInsides(const Material &material) {
        constexpr uint32 CUBE_VERTEX_COUNT = 36;

        constexpr float NEG_ONE_F = -1.0f;
        constexpr float ONE_F = 1.0f;

        constexpr uint16 ZERO = 0;
        constexpr uint16 ONE_UI = 0xffff - 1;

        Mesh::Vertex cubeVertices[CUBE_VERTEX_COUNT] = {
            //Front
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, {  NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },

            //Right
            { { ONE_F, ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ONE_UI } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ZERO, ONE_UI } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F, ONE_F }, { ONE_UI, ZERO } },

            //Left
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F, ONE_F }, { ZERO, ONE_UI } },

            //Back
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },

            //Bottom
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },

            //Top
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO} },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
        };

        return Mesh(cubeVertices, CUBE_VERTEX_COUNT, material);
    }

    Mesh Mesh::createHorizontalPlane(const Material &material) {
        constexpr uint32 VERTEX_COUNT = 6;

        constexpr float NEG_ONE_F = -1.0f;
        constexpr float ONE_F = 1.0f;

        constexpr uint16 ZERO = 0;
        constexpr uint16 ONE_UI = 0xffff - 1;

        Mesh::Vertex vertices[VERTEX_COUNT] = {
            { { NEG_ONE_F, ZERO, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ZERO, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ZERO, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ZERO, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ZERO, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ZERO , NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },
        };

        return Mesh(vertices, VERTEX_COUNT, material);
    }

    Mesh::Mesh(const char *path, const Material &material) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string fullPath = Application::getInstance().getAssetsPath() + path;
        auto fullPathWithoutFileName = Utils::removeFileNameFromPath(fullPath);

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fullPath.c_str(), fullPathWithoutFileName.c_str())) {
            BZ_CRITICAL_ERROR_ALWAYS("Error loading mesh with path: {}. Error: {}", path, err);
        }

        if (!warn.empty()) {
            BZ_LOG_CORE_WARN("Mesh with path: {}. Warning: {}", path, warn);
        }

        std::vector<Vertex> vertices;
        std::vector<uint32> indices;
        std::unordered_map<Vertex, uint32_t> uniqueVertices;

        uint32 shapeIdx = 0;
        uint32 vxOffset= 0;
        uint32 idxOffset = 0;

        for (const auto& shape : shapes) {
            uint32 shapeVxCount = 0;
            uint32 shapeIdxCount = 0;

            for (const auto& index : shape.mesh.indices) {
                Vertex vertex = {};

                vertex.position[0] = attrib.vertices[3 * index.vertex_index + 0];
                vertex.position[1] = attrib.vertices[3 * index.vertex_index + 1];
                vertex.position[2] = attrib.vertices[3 * index.vertex_index + 2];

                if (!attrib.normals.empty()) {
                    vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
                    vertex.normal[1] = attrib.normals[3 * index.normal_index + 1];
                    vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];
                    vertex.normal = glm::normalize(vertex.normal);
                }

                if (!attrib.texcoords.empty()) {
                    constexpr float SHORT_MAX_FLOAT = static_cast<float>(0xffff);
                    vertex.texCoord[0] = static_cast<uint16>(attrib.texcoords[2 * index.texcoord_index + 0] * SHORT_MAX_FLOAT);
                    vertex.texCoord[1] = static_cast<uint16>(attrib.texcoords[2 * index.texcoord_index + 1] * SHORT_MAX_FLOAT);
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                    shapeVxCount++;
                }

                indices.push_back(uniqueVertices[vertex]);
                shapeIdxCount++;
            }

            SubMesh submesh;
            submesh.vertexOffset = vxOffset;
            submesh.vertexCount = shapeVxCount;
            submesh.indexOffset = idxOffset;
            submesh.indexCount = shapeIdxCount;

            vxOffset += shapeVxCount;
            idxOffset += shapeIdxCount;

            if (materials.empty()) {
                submesh.material = material;
            }
            else {
                //Assuming all faces of the same shape have the same material.
                tinyobj::material_t material = materials[shape.mesh.material_ids[0]];

                std::string pathWithoutFileName = Utils::removeFileNameFromPath(path);
                submesh.material = Material((pathWithoutFileName + material.diffuse_texname).c_str(),
                    !material.normal_texname.empty() ? (pathWithoutFileName + material.normal_texname).c_str() : nullptr,
                    !material.metallic_texname.empty() ? (pathWithoutFileName + material.metallic_texname).c_str() : nullptr,
                    !material.roughness_texname.empty() ? (pathWithoutFileName + material.roughness_texname).c_str() : nullptr,
                    !material.bump_texname.empty() ? (pathWithoutFileName + material.bump_texname).c_str() : nullptr,
                    !material.ambient_texname.empty() ? (pathWithoutFileName + material.ambient_texname).c_str() : nullptr);
            }
            submeshes.push_back(submesh);
            shapeIdx++;
        }

        vertexCount = static_cast<uint32>(vertices.size());
        indexCount = static_cast<uint32>(indices.size());

        //Only compute tangents if texcoords and normals are present.
        if (!attrib.texcoords.empty() && !attrib.normals.empty()) {
            computeTangents(vertices, indices);
        }
        else {
            BZ_LOG_CORE_WARN("Not computing tangents for mesh: {}. There are no texcoords or no normals.", path);
        }

        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        indexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * indexCount, MemoryType::GpuOnly, Renderer::getIndexDataLayout());

        vertexBuffer->setData(vertices.data(), sizeof(Vertex) * vertexCount, 0);
        indexBuffer->setData(indices.data(), sizeof(uint32) * indexCount, 0);
    }

    Mesh::Mesh(Vertex vertices[], uint32 vertexCount, const Material &material) :
        vertexCount(vertexCount), indexCount(0) {
        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        vertexBuffer->setData(vertices, sizeof(Vertex) * vertexCount, 0);

        SubMesh submesh;
        submesh.vertexOffset = 0;
        submesh.vertexCount = vertexCount;
        submesh.indexOffset = 0;
        submesh.indexCount = 0;
        submesh.material = material;
        submeshes.push_back(submesh);
    }

    Mesh::Mesh(Vertex vertices[], uint32 vertexCount, uint32 indices[], uint32 indexCount, const Material &material) :
        vertexCount(vertexCount), indexCount(indexCount) {
        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        indexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * indexCount, MemoryType::GpuOnly, Renderer::getIndexDataLayout());

        vertexBuffer->setData(vertices, sizeof(Vertex) * vertexCount, 0);
        indexBuffer->setData(indices, sizeof(uint32) * indexCount, 0);

        SubMesh submesh;
        submesh.vertexOffset = 0;
        submesh.vertexCount = vertexCount;
        submesh.indexOffset = 0;
        submesh.indexCount = indexCount;
        submesh.material = material;
        submeshes.push_back(submesh);
    }

    void Mesh::computeTangents(std::vector<Vertex> &vertices, const std::vector<uint32> &indices) {
        BZ_ASSERT(!vertices.empty() && !indices.empty(), "Vertices and Indices are needed to compute tangents!");

        std::vector<glm::vec3> tempTangents;
        std::vector<glm::vec3> tempBitangents;
        tempTangents.resize(vertexCount, { 0.0f, 0.0f, 0.0f });
        tempBitangents.resize(vertexCount, { 0.0f, 0.0f, 0.0f });

        for (int i = 0; i < indices.size(); i += 3) {
            Vertex &vertex1 = vertices[indices[i + 0]];
            Vertex &vertex2 = vertices[indices[i + 1]];
            Vertex &vertex3 = vertices[indices[i + 2]];

            glm::vec3 edge1 = vertex2.position - vertex1.position;
            glm::vec3 edge2 = vertex3.position - vertex1.position;

            constexpr float SHORT_MAX_FLOAT = static_cast<float>(0xffff);
            glm::vec2 deltaUV1 = (glm::vec2(vertex2.texCoord[0], vertex2.texCoord[1]) - glm::vec2(vertex1.texCoord[0], vertex1.texCoord[1])) / SHORT_MAX_FLOAT;
            glm::vec2 deltaUV2 = (glm::vec2(vertex3.texCoord[0], vertex3.texCoord[1]) - glm::vec2(vertex1.texCoord[0], vertex1.texCoord[1])) / SHORT_MAX_FLOAT;

            glm::vec3 tangent = deltaUV2.t * edge1 - deltaUV1.t * edge2;
            tangent = glm::normalize(tangent);

            glm::vec3 bitangent = -deltaUV2.s * edge1 + deltaUV1.s * edge2;
            bitangent = glm::normalize(bitangent);

            tempTangents[indices[i + 0]] += tangent;
            tempTangents[indices[i + 1]] += tangent;
            tempTangents[indices[i + 2]] += tangent;

            tempBitangents[indices[i + 0]] += bitangent;
            tempBitangents[indices[i + 1]] += bitangent;
            tempBitangents[indices[i + 2]] += bitangent;
        }

        for (uint32 i = 0; i < vertexCount; ++i) {
            Vertex &v = vertices[i];
            glm::vec3 tangent = glm::normalize(tempTangents[i]);
            glm::vec3 bitangent = glm::normalize(tempBitangents[i]);

            //Ensure tangent is perpendicular to normal (Gram-Schmidt).
            tangent -= v.normal * glm::dot(tangent, v.normal);
            glm::vec3 orthTangent = glm::normalize(tangent);
            v.tangentAndDet.x = orthTangent.x;
            v.tangentAndDet.y = orthTangent.y;
            v.tangentAndDet.z = orthTangent.z;

            if (glm::dot(glm::cross(v.normal, orthTangent), bitangent) < 0.0f) {
                v.tangentAndDet.w = -1.0f;
            }
            else {
                v.tangentAndDet.w = 1.0f;
            }

            //Ensure bitangent is perpendicular to tangent and normal (Gram-Schmidt).
            //bitangent -= v.normal * glm::dot(bitangent, v.normal);
            //bitangent -= v.tangent * glm::dot(bitangent, v.tangent);
            //v.bitangent = glm::normalize(bitangent);
        }
    }
}