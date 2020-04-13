#pragma once


namespace BZ {

    class Transform;
    class Mesh;
    class Material;
    class Scene;
    class PipelineState;
    class DataLayout;
    class DescriptorSetLayout;
    class Buffer;
    class Entity;
    class DescriptorSet;
    class Sampler;

    struct RendererStats {
        uint32 vertexCount;
        uint32 triangleCount;
        uint32 drawCallCount;
        uint32 materialCount;
    };

    class Renderer {
    public:
        static void drawScene(const Scene &scene);

        static const RendererStats& getStats() { return stats; }

        static const DataLayout& getVertexDataLayout();
        static const DataLayout& getIndexDataLayout();

        //Pre-filled DescriptorSets to be used on Scenes and Materials. They will fill the remaining bindings.
        static Ref<DescriptorSet> createSceneDescriptorSet();
        static Ref<DescriptorSet> createMaterialDescriptorSet();

        static Ref<Sampler> getDefaultSampler();

    private:
        friend class Application;

        static void init();
        static void destroy();

        static void drawEntity(const Entity &entity, uint32 index);

        static void drawMesh(const Ref<PipelineState> &pipelineState, const Mesh &mesh, const Transform &transform);
        //static void drawMesh(const Ref<PipelineState> &pipelineState, const Mesh &mesh, const Transform &transform, const Material &fallbackMaterial);

        //static void drawSkyBox(const SkyBox &skyBox);

        static void handleMaterial(const Material &material);

        static RendererStats stats;
    };
}

