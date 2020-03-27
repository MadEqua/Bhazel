#include "bzpch.h"

#include "Mesh.h"
#include "Core/Application.h"
#include "Core/Utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


namespace BZ {

    static DataLayout vertexLayout = {
        { DataType::Float32, DataElements::Vec3, "POSITION" },
        { DataType::Float32, DataElements::Vec3, "NORMAL" },
        { DataType::Float32, DataElements::Vec3, "TANGENT" },
        { DataType::Uint16, DataElements::Vec2, "TEXCOORD", true },
    };

    static DataLayout indexLayout = {
        { DataType::Uint32, DataElements::Scalar, "" }
    };


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

        vertexBuffer = Buffer::create(BufferType::Vertex, sizeof(Vertex) * static_cast<uint32>(vertices.size()), MemoryType::GpuOnly, vertexLayout);
        indexBuffer = Buffer::create(BufferType::Index, sizeof(uint32) * static_cast<uint32>(indices.size()), MemoryType::GpuOnly, indexLayout);

        vertexBuffer->setData(vertices.data(), sizeof(Vertex) * static_cast<uint32>(vertices.size()), 0);
        indexBuffer->setData(indices.data(), sizeof(uint32) * static_cast<uint32>(indices.size()), 0);
    }
}