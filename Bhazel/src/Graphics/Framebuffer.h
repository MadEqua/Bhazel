#pragma once


namespace BZ {

    class TextureView;
    class RenderPass;

    class Framebuffer {
    public:
        static Ref<Framebuffer> create(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensionsAndLayers);

        const glm::ivec3& getDimensionsAndLayers() const { return dimensionsAndLayers; }
        const Ref<RenderPass>& getRenderPass() const { return renderPass; }

        const Ref<TextureView>& getDepthStencilTextureView() const { return depthStencilTextureView; }

    protected:
        explicit Framebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions);
        virtual ~Framebuffer() = default;

        Ref<RenderPass> renderPass;

        //z = layers
        glm::ivec3 dimensionsAndLayers;

        //In the same order of the RenderPass AttachmentDescriptors.
        std::vector<Ref<TextureView>> colorTextureViews;

        //Can be Depth only or DepthStencil
        Ref<TextureView> depthStencilTextureView;
    };
}