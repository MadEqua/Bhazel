#include "bzpch.h"

#include "Framebuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Graphics/RenderPass.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"


namespace BZ {

    Ref<Framebuffer> Framebuffer::create(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensionsAndLayers) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanFramebuffer>(renderPass, textureViews, dimensionsAndLayers);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Framebuffer::Framebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensionsAndLayers) :
        renderPass(renderPass), dimensionsAndLayers(dimensionsAndLayers) {

        BZ_ASSERT_CORE(textureViews.size() == renderPass->getAttachmentCount(), "The number of TextureViews must match the number of Attachments declared on the RenderPass!");
        
        uint32 colorIndex = 0;
        for (const auto &texView : textureViews) {
            if (texView->getTextureFormat().isColor()) {
                const auto &desc = renderPass->getColorAttachmentDescription(colorIndex);
                BZ_ASSERT_CORE(texView->getTextureFormat() == desc.format, "TextureView format is not compatible with RenderPass!");
                colorTextureViews.push_back(texView);
                colorIndex++;
            }
            else if (texView->getTextureFormat().isDepth()) {
                BZ_ASSERT_CORE(!depthStencilTextureView, "Can't have more than one DepthStencil TextureView!");
                BZ_ASSERT_CORE(renderPass->hasDepthStencilAttachment(), "The RenderPass doesn't declare any DepthStencil Attachment!");
                const auto desc = renderPass->getDepthStencilAttachmentDescription();
                BZ_ASSERT_CORE(texView->getTextureFormat() == desc->format, "TextureView format is not compatible with RenderPass!");
                depthStencilTextureView = texView;
            }
        }
    }
}