#pragma once

#include "Graphics/Buffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Material.h"


namespace BZ {

    class Mesh {

    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec3 bitangent;
            uint16 texCoord[2];

            bool operator==(const Vertex& other) const {
                return memcmp(this, &other, sizeof(Vertex)) == 0;
            }
        };

        static Mesh createUnitCube(const Material &material = Material());
        static Mesh createUnitCubeInsides(const Material &material = Material());

        Mesh() = default;
        explicit Mesh(const char *path, const Material &material = Material());
        Mesh(Vertex vertices[], uint32 vertexCount, const Material &material = Material());
        Mesh(Vertex vertices[], uint32 vertexCount, uint32 indices[], uint32 indexCount, const Material &material = Material());

        const Ref<Buffer>& getVertexBuffer() const { return vertexBuffer; }
        const Ref<Buffer>& getIndexBuffer() const { return indexBuffer; }

        uint32 getVertexCount() const { return vertexCount; }
        uint32 getIndexCount() const { return indexCount; }

        bool isValid() const { return vertexCount > 0 && static_cast<bool>(vertexBuffer); }
        bool hasIndices() const { return indexCount > 0; }

        const Material& getMaterial() const { return material; }

    private:
        uint32 vertexCount;
        uint32 indexCount;

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        Material material;

        void computeTangents(std::vector<Vertex> &vertices, const std::vector<uint32> &indices);
    };
}

template<> struct std::hash<BZ::Mesh::Vertex> {
    size_t operator()(const BZ::Mesh::Vertex &vertex) const {
        return ((std::hash<glm::vec3>()(vertex.position) ^
            (std::hash<glm::vec3>()(vertex.normal) << 1) ^
            (std::hash<glm::vec3>()(vertex.tangent) << 1) ^
            (std::hash<glm::vec3>()(vertex.bitangent) << 1)) >> 1) ^
            (vertex.texCoord[0] ^ vertex.texCoord[1]);
    }
};