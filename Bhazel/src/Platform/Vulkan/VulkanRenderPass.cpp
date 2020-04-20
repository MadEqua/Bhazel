#include "bzpch.h"

#include "VulkanRenderPass.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    VulkanRenderPass::VulkanRenderPass(const std::initializer_list<AttachmentDescription> &descs) :
        RenderPass(descs) {

        init(descs, false);
        init(descs, true);
    }

    VulkanRenderPass::~VulkanRenderPass() {
        vkDestroyRenderPass(getDevice(), nativeHandle.original, nullptr);
        vkDestroyRenderPass(getDevice(), nativeHandle.forceClear, nullptr);
    }


    void VulkanRenderPass::init(const std::initializer_list<AttachmentDescription> &descs, bool forceClear) {

        std::vector<VkAttachmentDescription> attachmentDescriptions(getAttachmentCount());
        std::vector<VkAttachmentReference> colorAttachmentRefs(getColorAttachmentCount());

        int i = 0;
        for (const auto &attDesc : colorAttachmentDescs) {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.format = textureFormatToVk(attDesc.format);
            attachmentDesc.samples = sampleCountToVk(attDesc.samples);
            attachmentDesc.loadOp = forceClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : loadOperationToVk(attDesc.loadOperatorColorAndDepth);
            attachmentDesc.storeOp = storeOperationToVk(attDesc.storeOperatorColorAndDepth);
            attachmentDesc.stencilLoadOp = loadOperationToVk(attDesc.loadOperatorStencil);
            attachmentDesc.stencilStoreOp = storeOperationToVk(attDesc.storeOperatorStencil);
            attachmentDesc.initialLayout = textureLayoutToVk(attDesc.initialLayout);
            attachmentDesc.finalLayout = textureLayoutToVk(attDesc.finalLayout);
            attachmentDescriptions[i] = attachmentDesc;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRef.attachment = i;
            colorAttachmentRefs[i] = colorAttachmentRef;

            i++;
        }

        if (depthStencilAttachmentDesc) {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.format = textureFormatToVk(depthStencilAttachmentDesc->format);
            attachmentDesc.samples = sampleCountToVk(depthStencilAttachmentDesc->samples);
            attachmentDesc.loadOp = forceClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : loadOperationToVk(depthStencilAttachmentDesc->loadOperatorColorAndDepth);
            attachmentDesc.storeOp = storeOperationToVk(depthStencilAttachmentDesc->storeOperatorColorAndDepth);
            attachmentDesc.stencilLoadOp = loadOperationToVk(depthStencilAttachmentDesc->loadOperatorStencil);
            attachmentDesc.stencilStoreOp = storeOperationToVk(depthStencilAttachmentDesc->storeOperatorStencil);
            attachmentDesc.initialLayout = textureLayoutToVk(depthStencilAttachmentDesc->initialLayout);
            attachmentDesc.finalLayout = textureLayoutToVk(depthStencilAttachmentDesc->finalLayout);
            attachmentDescriptions[i] = attachmentDesc;
        }

        VkAttachmentReference depthStencilAttachmentRef;
        depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentRef.attachment = i;

        //Create subpass
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = getColorAttachmentCount();
        subpassDesc.pColorAttachments = colorAttachmentRefs.data(); //The index of the attachment in this array is directly referenced from the fragment shader
        subpassDesc.inputAttachmentCount = 0;
        subpassDesc.pInputAttachments = nullptr;
        subpassDesc.pResolveAttachments = nullptr;
        subpassDesc.pDepthStencilAttachment = hasDepthStencilAttachment() ? &depthStencilAttachmentRef : nullptr;
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

        BZ_ASSERT_VK(vkCreateRenderPass(getDevice(), &renderPassInfo, nullptr, forceClear ? &nativeHandle.forceClear : &nativeHandle.original));
    }
}