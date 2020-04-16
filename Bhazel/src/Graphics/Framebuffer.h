#pragma once


namespace BZ {

    class TextureView;
    class RenderPass;

    class Framebuffer {
    public:
        static Ref<Framebuffer> create(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions);

        const glm::ivec3& getDimensions() const { return dimensions; }
        const Ref<RenderPass>& getRenderPass() const { return renderPass; }

    protected:
        explicit Framebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions);
        virtual ~Framebuffer() = default;

        Ref<RenderPass> renderPass;

        glm::ivec3 dimensions;

        //In the same order of the RenderPass AttachmentDescriptors.
        std::vector<Ref<TextureView>> colorTextureViews;

        //Can be Depth only or DepthStencil
        Ref<TextureView> depthStencilTextureView;
    };
}