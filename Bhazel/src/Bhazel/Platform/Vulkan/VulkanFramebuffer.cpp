#include "bzpch.h"

#include "VulkanFramebuffer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"
#include "Bhazel/Platform/Vulkan/VulkanTexture.h"


namespace BZ {

    VulkanFramebuffer::VulkanFramebuffer(const std::vector<Ref<TextureView>> &textureViews) :
        context(static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext())),
        Framebuffer(textureViews) {

        //TODO: this will create identical render passes. have a pool?
        initRenderPass();

        std::vector<VkImageView> vkImageViews(textureViews.size());
        for(int i = 0; i < textureViews.size(); ++i) {
            const VulkanTextureView &vkTextureView = static_cast<const VulkanTextureView &>(*textureViews[i]); //TODO: bad
            vkImageViews[i] = vkTextureView.imageViewHandle;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassHandle;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
        framebufferInfo.pAttachments = vkImageViews.data();
        framebufferInfo.width = textureViews[0]->getTexture()->getWidth();
        framebufferInfo.height = textureViews[0]->getTexture()->getHeight();
        framebufferInfo.layers = textureViews[0]->getTexture()->getDepth();

        BZ_ASSERT_VK(vkCreateFramebuffer(context.getDevice(), &framebufferInfo, nullptr, &framebufferHandle));
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        vkDestroyFramebuffer(context.getDevice(), framebufferHandle, nullptr);
        vkDestroyRenderPass(context.getDevice(), renderPassHandle, nullptr);
    }

    void VulkanFramebuffer::initRenderPass() {
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        //Create subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef; //The index of the attachment in this array is directly referenced from the fragment shader

        //Create render pass
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = textureFormatToVk(textureViews[0]->getTexture()->getFormat());
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //TODO

        //For synchronization with the swapchain framebuffer/image
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        BZ_ASSERT_VK(vkCreateRenderPass(context.getDevice(), &renderPassInfo, nullptr, &renderPassHandle));
    }
}