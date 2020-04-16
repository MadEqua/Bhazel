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
    class Framebuffer;
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
        static Ref<Framebuffer> createShadowMapFramebuffer();

        static const Ref<Sampler>& getDefaultSampler();

    private:
        friend class Application;

        static void init();
        static void destroy();

        static void depthPass(const Scene &scene);
        static void colorPass(const Scene &scene);

        static void drawEntity(const Entity &entity, uint32 index);
        static void drawMesh(const Mesh &mesh, const Transform &transform);

        static void handleEntities(const Scene &scene);
        static void handleMaterials(const Scene &scene);
        static void handleMaterial(const Material &material);

        static RendererStats stats;
    };
}

