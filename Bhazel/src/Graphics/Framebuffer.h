#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    class TextureView;
    class RenderPass;

    class Framebuffer : public GpuObject<VkFramebuffer> {
    public:
        static Ref<Framebuffer> create(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensionsAndLayers);

        Framebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions);
        ~Framebuffer();

        BZ_NON_COPYABLE(Framebuffer);

        const glm::ivec3& getDimensionsAndLayers() const { return dimensionsAndLayers; }
        const Ref<RenderPass>& getRenderPass() const { return renderPass; }

        const Ref<TextureView>& getDepthStencilTextureView() const { return depthStencilTextureView; }

    private:
        Ref<RenderPass> renderPass;

        //z = layers
        glm::ivec3 dimensionsAndLayers;

        //In the same order of the RenderPass AttachmentDescriptors.
        std::vector<Ref<TextureView>> colorTextureViews;

        //Can be Depth only or DepthStencil
        Ref<TextureView> depthStencilTextureView;
    };
}