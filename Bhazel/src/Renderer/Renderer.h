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
    class TextureView;
    struct FrameStats;


    class Renderer {
    public:
        static void drawScene(const Scene &scene);

        static const DataLayout& getVertexDataLayout();
        static const DataLayout& getIndexDataLayout();

        //Pre-filled DescriptorSets to be used on Scenes and Materials. They will fill the remaining bindings.
        static Ref<DescriptorSet> createSceneDescriptorSet();
        static Ref<DescriptorSet> createMaterialDescriptorSet();
        static Ref<Framebuffer> createShadowMapFramebuffer();

        static const Ref<Sampler>& getDefaultSampler();
        static const Ref<Sampler>& getShadowSampler();

        static const Ref<TextureView>& getDummyTextureView();

    private:
        friend class Application;

        static void init();
        static void destroy();

        static void depthPass(const Scene &scene);
        static void colorPass(const Scene &scene);

        static void drawEntity(const Entity &entity, uint32 index);
        static void drawMesh(const Mesh &mesh, const Material &overrideMaterial);

        static void fillConstants(const Scene &scene);
        static void fillPasses(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices);
        static void fillScene(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices);
        static void fillEntities(const Scene &scene);
        static void fillMaterials(const Scene &scene);
        static void fillMaterial(const Material &material);

        static void onImGuiRender(const FrameStats &frameStats);
    };
}

