#pragma once

#include "Graphics/Buffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace BZ {

    class Mesh {

    public:
        //TODO: create method that returns a Ref?
        Mesh() = default;
        Mesh(const char *path);

        //TODO: methods for adding vertices and indices

        const Ref<Buffer>& getVertexBuffer() const { return vertexBuffer; }
        const Ref<Buffer>& getIndexBuffer() const { return indexBuffer; }

        uint32 getVertexCount() const { return static_cast<uint32>(vertices.size()); }
        uint32 getIndexCount() const { return static_cast<uint32>(indices.size()); }

        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            uint16 texCoord[2];

            bool operator==(const Vertex& other) const {
                return memcmp(this, &other, sizeof(Vertex)) == 0;
            }
        };

    private:
        std::vector<Vertex> vertices;
        std::vector<uint32> indices;

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;
    };
}

template<> struct std::hash<BZ::Mesh::Vertex> {
    size_t operator()(const BZ::Mesh::Vertex &vertex) const {
        return ((std::hash<glm::vec3>()(vertex.position) ^
            (std::hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
            (std::hash<glm::vec3>()(vertex.tangent) << 1) ^
            (vertex.texCoord[0] ^ vertex.texCoord[1]);
    }
};