#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Material.h"


namespace BZ {

    class Buffer;

    class Mesh {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec4 tangentAndDet;
            uint16 texCoord[2];

            bool operator==(const Vertex& other) const {
                return memcmp(this, &other, sizeof(Vertex)) == 0;
            }
        };

        struct SubMesh {
            Material material;
            uint32 vertexOffset;
            uint32 vertexCount;
            uint32 indexOffset;
            uint32 indexCount;
        };

        static Mesh createUnitCube(const Material &material = Material());
        static Mesh createUnitCubeInsides(const Material &material = Material());
        static Mesh createHorizontalPlane(const Material &material = Material());

        Mesh() = default;
        explicit Mesh(const char *path, const Material &material = Material());
        Mesh(Vertex vertices[], uint32 vertexCount, const Material &material = Material());
        Mesh(Vertex vertices[], uint32 vertexCount, uint32 indices[], uint32 indexCount, const Material &material = Material());

        const Ref<Buffer>& getVertexBuffer() const { return vertexBuffer; }
        const Ref<Buffer>& getIndexBuffer() const { return indexBuffer; }

        uint32 getVertexCount() const { return vertexCount; }
        uint32 getIndexCount() const { return indexCount; }

        bool isValid() const { return vertexCount > 0 && static_cast<bool>(vertexBuffer) && !submeshes.empty(); }
        bool hasIndices() const { return indexCount > 0; }

        uint32 getSubMeshCount() const { return static_cast<uint32>(submeshes.size()); }
        const SubMesh& getSubMeshIdx(uint32 idx) const { BZ_ASSERT_CORE(idx < submeshes.size(), "Invalid index!"); return submeshes[idx]; }
        SubMesh& getSubMeshIdx(uint32 idx) { BZ_ASSERT_CORE(idx < submeshes.size(), "Invalid index!"); return submeshes[idx]; }
        const std::vector<SubMesh>& getSubmeshes() const { return submeshes; }
        std::vector<SubMesh>& getSubmeshes() { return submeshes; }

    private:
        uint32 vertexCount;
        uint32 indexCount;

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;

        std::vector<SubMesh> submeshes;

        void computeTangents(std::vector<Vertex> &vertices, const std::vector<uint32> &indices);
    };
}

template<> struct std::hash<BZ::Mesh::Vertex> {
    size_t operator()(const BZ::Mesh::Vertex &vertex) const {
        return std::hash<glm::vec3>()(vertex.position) ^
               std::hash<glm::vec3>()(vertex.normal) ^
               std::hash<glm::vec4>()(vertex.tangentAndDet) ^
               vertex.texCoord[0] ^ vertex.texCoord[1];
    }
};