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

        const Ref<TextureView>& getDepthStencilTextureView() const { return depthStencilTextureView; }
        uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorTextureViews.size()); }

    private:
        //z = layers
        glm::ivec3 dimensionsAndLayers;

        //In the same order of the RenderPass AttachmentDescriptors.
        std::vector<Ref<TextureView>> colorTextureViews;

        //Can be Depth only or DepthStencil
        Ref<TextureView> depthStencilTextureView;
    };
}