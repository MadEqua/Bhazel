#pragma once

#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"


namespace BZ {

    class Camera;
    class Transform;
    class Mesh;
    class Material;

    struct RendererStats {
        uint32 vertexCount;
        uint32 drawCallCount;
        uint32 descriptorSetBindCount;
    };

    class Renderer {
    public:
        static void beginScene(const Camera &camera);
        static void endScene();

        static void drawCube(const Transform &transform, const Material &material);

        static void drawMesh(const Mesh &mesh, const Transform &transform);
        static void drawMesh(const Mesh &mesh, const Transform &transform, const Material &fallbackMaterial);

        static const RendererStats& getStats() { return stats; }

        static DataLayout& getVertexDataLayout();
        static DataLayout& getIndexDataLayout();

        static Ref<DescriptorSetLayout>& getMaterialDescriptorSetLayout();
        static Ref<Sampler>& getDefaultSampler();

    private:
        friend class Application;

        static void init();
        static void destroy();

        static RendererStats stats;
    };
}

