#pragma once

#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"


namespace BZ {

    class Camera;
    class Transform;
    class Mesh;
    class Material;
    class Scene;

    struct RendererStats {
        uint32 vertexCount;
        uint32 triangleCount;
        uint32 drawCallCount;
    };

    class Renderer {
    public:
        static void drawScene(const Scene &scene);

        static const RendererStats& getStats() { return stats; }

        static DataLayout& getVertexDataLayout();
        static DataLayout& getIndexDataLayout();

        static Ref<DescriptorSetLayout>& getMaterialDescriptorSetLayout();
        static Ref<Sampler>& getDefaultSampler();

    private:
        friend class Application;

        static void init();
        static void destroy();

        //TODO
        //static void drawEntity();

        static void drawCube(const Transform &transform, const Material &material);

        static void drawMesh(const Mesh &mesh, const Transform &transform);
        static void drawMesh(const Mesh &mesh, const Transform &transform, const Material &fallbackMaterial);

        static RendererStats stats;
    };
}

