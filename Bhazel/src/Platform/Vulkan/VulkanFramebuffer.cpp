#include "bzpch.h"

#include "VulkanFramebuffer.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanTexture.h"


namespace BZ {

    VulkanFramebuffer::VulkanFramebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions) :
        Framebuffer(renderPass, textureViews, dimensions) {

        std::vector<VkImageView> vkImageViews(textureViews.size());

        //First color attachments, then depthstencil, if applicable.
        uint32 i = 0;
        for (const auto &colorTexView : colorTextureViews) {
            vkImageViews[i] = static_cast<VulkanTextureView&>(*colorTexView).getNativeHandle();
            i++;
        }
        
        if(depthStencilTextureView)
            vkImageViews[i] = static_cast<VulkanTextureView &>(*depthStencilTextureView).getNativeHandle();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = static_cast<VulkanRenderPass&>(*renderPass).getNativeHandle().original;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
        framebufferInfo.pAttachments = vkImageViews.data();
        framebufferInfo.width = dimensions.x;
        framebufferInfo.height = dimensions.y;
        framebufferInfo.layers = dimensions.z;

        BZ_ASSERT_VK(vkCreateFramebuffer(getDevice(), &framebufferInfo, nullptr, &nativeHandle));
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        vkDestroyFramebuffer(getDevice(), nativeHandle, nullptr);
    }
}