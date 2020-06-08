#include "bzpch.h"

#include "Framebuffer.h"

#include "Core/Application.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"


namespace BZ {

Ref<Framebuffer> Framebuffer::create(const Ref<RenderPass> &renderPass,
                                     const std::initializer_list<Ref<TextureView>> &textureViews,
                                     const glm::uvec3 &dimensionsAndLayers) {
    return MakeRef<Framebuffer>(renderPass, textureViews, dimensionsAndLayers);
}

Framebuffer::Framebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews,
                         const glm::uvec3 &dimensionsAndLayers) :
    dimensionsAndLayers(dimensionsAndLayers) {

    BZ_ASSERT_CORE(textureViews.size() == renderPass->getAttachmentCount(),
                   "The number of TextureViews must match the number of Attachments declared on the RenderPass!");

    uint32 colorIndex = 0;
    for (const auto &texView : textureViews) {
        if (texView->getTextureFormat().isColor()) {
            const auto &desc = renderPass->getColorAttachmentDescription(colorIndex);
            colorTextureViews.push_back(texView);
            colorIndex++;
        }
        else if (texView->getTextureFormat().isDepth()) {
            BZ_ASSERT_CORE(!depthStencilTextureView, "Can't have more than one DepthStencil TextureView!");
            BZ_ASSERT_CORE(renderPass->hasDepthStencilAttachment(),
                           "The RenderPass doesn't declare any DepthStencil Attachment!");
            const auto desc = renderPass->getDepthStencilAttachmentDescription();
            depthStencilTextureView = texView;
        }
    }

    std::vector<VkImageView> vkImageViews(textureViews.size());

    // First color attachments, then depthstencil, if applicable.
    uint32 i = 0;
    for (const auto &colorTexView : colorTextureViews) {
        vkImageViews[i] = colorTexView->getHandle();
        i++;
    }

    if (depthStencilTextureView) {
        vkImageViews[i] = depthStencilTextureView->getHandle();
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
    framebufferInfo.pAttachments = vkImageViews.data();
    framebufferInfo.width = dimensionsAndLayers.x;
    framebufferInfo.height = dimensionsAndLayers.y;
    framebufferInfo.layers = dimensionsAndLayers.z;

    BZ_ASSERT_VK(vkCreateFramebuffer(BZ_GRAPHICS_DEVICE.getHandle(), &framebufferInfo, nullptr, &handle));
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(BZ_GRAPHICS_DEVICE.getHandle(), handle, nullptr);
}

const Ref<TextureView> &Framebuffer::getColorAttachmentTextureView(uint32 index) const {
    BZ_ASSERT_CORE(index < getColorAttachmentCount(), "Index {} is out of range!", index);
    return colorTextureViews[index];
}
}