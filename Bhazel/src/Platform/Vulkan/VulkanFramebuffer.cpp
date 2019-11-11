#include "bzpch.h"

#include "VulkanFramebuffer.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    VulkanFramebuffer::VulkanFramebuffer(const Builder &builder) :
        Framebuffer(builder) {

        //TODO: this will create identical render passes. have a table?
        originalrenderPass.init(getDevice(), colorAttachments, depthStencilAttachment, false);
        clearRenderPass.init(getDevice(), colorAttachments, depthStencilAttachment, true);

        std::vector<VkImageView> vkImageViews(builder.attachments.size());

        //First color attachments, then depthstencil, if applicable.
        uint32 i;
        for(i = 0; i < getColorAttachmentCount(); ++i)
            vkImageViews[i] = static_cast<VulkanTextureView&>(*builder.attachments[i].textureView).getNativeHandle();
        
        if(hasDepthStencilAttachment())
            vkImageViews[i] = static_cast<VulkanTextureView &>(*builder.attachments[i].textureView).getNativeHandle();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = originalrenderPass.getNativeHandle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
        framebufferInfo.pAttachments = vkImageViews.data();
        framebufferInfo.width = dimensions.x;
        framebufferInfo.height = dimensions.y;
        framebufferInfo.layers = dimensions.z;

        BZ_ASSERT_VK(vkCreateFramebuffer(getDevice(), &framebufferInfo, nullptr, &nativeHandle));
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        vkDestroyFramebuffer(getDevice(), nativeHandle, nullptr);
        originalrenderPass.destroy();
        clearRenderPass.destroy();
    }
}