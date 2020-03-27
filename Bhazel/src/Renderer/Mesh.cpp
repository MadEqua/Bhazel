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
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ZERO, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ONE_UI } },

            //Right
            { { ONE_F, NEG_ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, ONE_F }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO, NEG_ONE_F }, { ZERO, ONE_UI } },

            //Left
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ZERO, ZERO } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO, ONE_F }, { ZERO, ONE_UI } },

            //Back
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ZERO } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ZERO, NEG_ONE_F }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ONE_UI } },

            //Bottom
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, NEG_ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, NEG_ONE_F, ONE_F }, { ZERO, NEG_ONE_F, ZERO }, { NEG_ONE_F, ZERO, ZERO }, { ZERO, ONE_UI } },

            //Top
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ONE_UI, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ZERO, ZERO } },
            { { ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ONE_UI, ONE_UI } },
            { { NEG_ONE_F, ONE_F, NEG_ONE_F }, { ZERO, ONE_F, ZERO }, { ONE_F, ZERO, ZERO }, { ZERO, ONE_UI } },
        };

        return Mesh(cubeVertices, CUBE_VERTEX_COUNT, material);
    }

    Mesh::Mesh(const char *path) {
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

        for (const auto& shape : shapes) {
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

                //vertex.tangent[0] = attrib.tangents[3 * index.tangent_index + 0];
                //vertex.tangent[1] = attrib.tangents[3 * index.tangent_index + 1];
                //vertex.tangent[2] = attrib.tangents[3 * index.tangent_index + 2];

                if (!attrib.texcoords.empty()) {
                    constexpr float MAX = static_cast<float>(0xffff);
                    vertex.texCoord[0] = static_cast<uint16>(attrib.texcoords[2 * index.texcoord_index + 0] * MAX);
                    vertex.texCoord[1] = static_cast<uint16>(attrib.texcoords[2 * index.texcoord_index + 1] * MAX);
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        auto pathWithoutFileName = Utils::removeFileNameFromPath(path);
        for (auto &material : materials) {
            this->material = Material((pathWithoutFileName + material.diffuse_texname).c_str());
        }

        vertexCount = static_cast<uint32>(vertices.size());
        indexCount = static_cast<uint32>(indices.size());

        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        indexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * indexCount, MemoryType::GpuOnly, Renderer::getIndexDataLayout());

        vertexBuffer->setData(vertices.data(), sizeof(Vertex) * vertexCount, 0);
        indexBuffer->setData(indices.data(), sizeof(uint32) * indexCount, 0);
    }

    Mesh::Mesh(Vertex vertices[], uint32 vertexCount, const Material &material) :
        vertexCount(vertexCount), indexCount(0), material(material) {
        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        vertexBuffer->setData(vertices, sizeof(Vertex) * vertexCount, 0);
    }

    Mesh::Mesh(Vertex vertices[], uint32 vertexCount, uint32 indices[], uint32 indexCount, const Material &material) :
        vertexCount(vertexCount), indexCount(indexCount), material(material) {
        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * vertexCount, MemoryType::GpuOnly, Renderer::getVertexDataLayout());
        indexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * indexCount, MemoryType::GpuOnly, Renderer::getIndexDataLayout());

        vertexBuffer->setData(vertices, sizeof(Vertex) * vertexCount, 0);
        indexBuffer->setData(indices, sizeof(uint32) * indexCount, 0);
    }
}