#include "bzpch.h"

#include "VulkanRenderPass.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"


namespace BZ {

    void VulkanRenderPass::init(VkDevice device, const std::vector<Framebuffer::Attachment> &colorAttachments, const std::optional<Framebuffer::Attachment> &depthStencilAttachment, bool forceClearAttachments) {
        this->device = device;

        BZ_ASSERT_CORE(nativeHandle == VK_NULL_HANDLE, "RenderPass is already inited!");

        std::vector<VkAttachmentDescription> attachmentDescriptions(colorAttachments.size() + (depthStencilAttachment.has_value() ? 1 : 0));
        int i = 0;
        for(const auto &att : colorAttachments) {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.format = textureFormatToVk(att.description.format);
            attachmentDesc.samples = sampleCountToVk(att.description.samples);
            attachmentDesc.loadOp = forceClearAttachments ? VK_ATTACHMENT_LOAD_OP_CLEAR : loadOperationToVk(att.description.loadOperatorColorAndDepth);
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
            attachmentDesc.loadOp = forceClearAttachments ? VK_ATTACHMENT_LOAD_OP_CLEAR : loadOperationToVk(depthStencilAttachment->description.loadOperatorColorAndDepth);
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
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRef.attachment = i;
            colorAttachmentRefs[i++] = colorAttachmentRef;
        }

        VkAttachmentReference depthStencilAttachmentRef;
        depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentRef.attachment = i;

        //Create subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = static_cast<uint32>(colorAttachments.size());
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
        renderPassInfo.attachmentCount = static_cast<uint32>(attachmentDescriptions.size());
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1; //Harcoded to 1 subpass
        renderPassInfo.pSubpasses = &subpassDesc;
        renderPassInfo.dependencyCount = 0;
        renderPassInfo.pDependencies = nullptr;

        BZ_ASSERT_VK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &nativeHandle));
    }

    void VulkanRenderPass::destroy() {
        vkDestroyRenderPass(device, nativeHandle, nullptr);
        nativeHandle = VK_NULL_HANDLE;
    }
}