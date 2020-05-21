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
    class RenderPass;
    class Framebuffer;
    class Sampler;
    class TextureView;
    class CommandBuffer;
    struct FrameStats;


    class Renderer {
    public:
        static void renderScene(const Scene &scene);

        static const DataLayout& getVertexDataLayout();
        static const DataLayout& getIndexDataLayout();

        //Pre-filled DescriptorSets to be used on Scenes and Materials. They will fill the remaining bindings.
        static DescriptorSet& createSceneDescriptorSet();
        static DescriptorSet& createMaterialDescriptorSet();
        static Ref<Framebuffer> createShadowMapFramebuffer();

        static const Ref<Sampler>& getDefaultSampler();
        static const Ref<Sampler>& getShadowSampler();

    private:
        friend class RendererCoordinator;
        friend class Application;

        static void init();
        static void destroy();

        static void initShadowPassData();
        static void initColorPassData();
        static void initSkyBoxData();

        static void shadowPass(const Scene &scene);
        static void colorPass(const Scene &scene);

        static void drawEntities(CommandBuffer &commandBuffer, const Scene &scene, bool shadowPass);
        static void drawMesh(CommandBuffer &commandBuffer, const Mesh &mesh, const Material &overrideMaterial, bool shadowPass);

        static void fillConstants(const Scene &scene);
        static void fillScene(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices, const float cascadeSplits[]);
        static void fillPasses(const Scene &scene, const glm::mat4 *lightMatrices, const glm::mat4 *lightProjectionMatrices);
        static void fillMaterials(const Scene &scene);
        static void fillMaterial(const Material &material);
        static void fillEntities(const Scene &scene);

        static void render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer, bool waitForImageAvailable, bool signalFrameEnd);
        static void onImGuiRender(const FrameStats &frameStats);

        static void computeCascadedShadowMappingSplits(float out[], uint32 splits, float nearPlane, float farPlane);
    };
}

