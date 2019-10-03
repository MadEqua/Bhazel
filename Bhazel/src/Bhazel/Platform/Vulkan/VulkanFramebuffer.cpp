#include "bzpch.h"

#include "VulkanFramebuffer.h"

#include "Bhazel/Platform/Vulkan/VulkanTexture.h"
#include "Bhazel/Platform/Vulkan/VulkanConversions.h"


namespace BZ {

    VulkanFramebuffer::VulkanFramebuffer(const std::vector<Attachment> &attachments, const glm::ivec3 &dimensions) :
        Framebuffer(attachments, dimensions) {

        //TODO: this will create identical render passes. have a pool?
        initRenderPass();

        std::vector<VkImageView> vkImageViews(attachments.size());
        for(int i = 0; i < attachments.size(); ++i) {
            vkImageViews[i] = static_cast<VulkanTextureView&>(*attachments[i].textureView).getNativeHandle();
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = nativeHandle.renderPassHandle;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
        framebufferInfo.pAttachments = vkImageViews.data();
        framebufferInfo.width = dimensions.x;
        framebufferInfo.height = dimensions.y;
        framebufferInfo.layers = dimensions.z;

        BZ_ASSERT_VK(vkCreateFramebuffer(getGraphicsContext().getDevice(), &framebufferInfo, nullptr, &nativeHandle.frameBufferHandle));
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        vkDestroyFramebuffer(getGraphicsContext().getDevice(), nativeHandle.frameBufferHandle, nullptr);
        vkDestroyRenderPass(getGraphicsContext().getDevice(), nativeHandle.renderPassHandle, nullptr);
    }

    void VulkanFramebuffer::initRenderPass() {

        std::vector<VkAttachmentDescription> attachmentDescriptions(colorAttachments.size() + (depthStencilAttachment ? 1 : 0));
        int i = 0;
        for(const auto &att : colorAttachments) {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.format = textureFormatToVk(att.description.format);
            attachmentDesc.samples = sampleCountToVk(att.description.samples);
            attachmentDesc.loadOp = loadOperationToVk(att.description.loadOperatorColorAndDepth);
            attachmentDesc.storeOp = storeOperationToVk(att.description.storeOperatorColorAndDepth);
            attachmentDesc.stencilLoadOp = loadOperationToVk(att.description.loadOperatorStencil);
            attachmentDesc.stencilStoreOp = storeOperationToVk(att.description.storeOperatorStencil);;
            attachmentDesc.initialLayout = textureLayoutToVk(att.description.initialLayout);
            attachmentDesc.finalLayout = textureLayoutToVk(att.description.finalLayout);
            attachmentDescriptions[i++] = attachmentDesc;
        }

        if(depthStencilAttachment) {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.format = textureFormatToVk(depthStencilAttachment->description.format);
            attachmentDesc.samples = sampleCountToVk(depthStencilAttachment->description.samples);
            attachmentDesc.loadOp = loadOperationToVk(depthStencilAttachment->description.loadOperatorColorAndDepth);
            attachmentDesc.storeOp = storeOperationToVk(depthStencilAttachment->description.storeOperatorColorAndDepth);
            attachmentDesc.stencilLoadOp = loadOperationToVk(depthStencilAttachment->description.loadOperatorStencil);
            attachmentDesc.stencilStoreOp = storeOperationToVk(depthStencilAttachment->description.storeOperatorStencil);;
            attachmentDesc.initialLayout = textureLayoutToVk(depthStencilAttachment->description.initialLayout);
            attachmentDesc.finalLayout = textureLayoutToVk(depthStencilAttachment->description.finalLayout);
            attachmentDescriptions[i] = attachmentDesc;
        }

        std::vector<VkAttachmentReference> colorAttachmentRefs(colorAttachments.size());
        i = 0;
        for(const auto &att : colorAttachments) {
            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = i;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs[i++] = colorAttachmentRef;
        }

        VkAttachmentReference depthStencilAttachmentRef;
        depthStencilAttachmentRef.attachment = i;
        depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //Create subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = getColorAttachmentCount();
        subpassDesc.pColorAttachments = colorAttachmentRefs.data(); //The index of the attachment in this array is directly referenced from the fragment shader
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.pDepthStencilAttachment = depthStencilAttachment ? &depthStencilAttachmentRef : nullptr;
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments = nullptr;

        //Create render pass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = getColorAttachmentCount() + (depthStencilAttachment ? 1 : 0);
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1; //Harcoded to 1 subpass
        renderPassInfo.pSubpasses = &subpassDesc;
        renderPassInfo.dependencyCount = 0;
        renderPassInfo.pDependencies = nullptr;

        BZ_ASSERT_VK(vkCreateRenderPass(getGraphicsContext().getDevice(), &renderPassInfo, nullptr, &nativeHandle.renderPassHandle));
    }
}