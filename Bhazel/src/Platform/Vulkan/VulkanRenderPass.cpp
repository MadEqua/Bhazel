#include "bzpch.h"

#include "VulkanRenderPass.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    VulkanRenderPass::VulkanRenderPass(const std::initializer_list<AttachmentDescription> &descs, 
                                       const std::initializer_list<SubPassDescription> &subPassDescs,
                                       const std::initializer_list<SubPassDependency> &subPassDeps) :
        RenderPass(descs, subPassDescs, subPassDeps) {

        init(false);
        init(true);
    }

    VulkanRenderPass::~VulkanRenderPass() {
        vkDestroyRenderPass(getDevice(), nativeHandle.original, nullptr);
        vkDestroyRenderPass(getDevice(), nativeHandle.forceClear, nullptr);
    }

    void VulkanRenderPass::init(bool forceClear) {

        std::vector<VkAttachmentDescription> vkAttachmentDescriptions(getAttachmentCount());

        int i = 0;
        for (const auto &attDesc : attachmentDescs) {
            VkAttachmentDescription vkAttachmentDesc = {};
            vkAttachmentDesc.format = textureFormatToVk(attDesc.format);
            vkAttachmentDesc.samples = sampleCountToVk(attDesc.samples);
            vkAttachmentDesc.loadOp = forceClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : loadOperationToVk(attDesc.loadOperatorColorAndDepth);
            vkAttachmentDesc.storeOp = storeOperationToVk(attDesc.storeOperatorColorAndDepth);
            vkAttachmentDesc.stencilLoadOp = loadOperationToVk(attDesc.loadOperatorStencil);
            vkAttachmentDesc.stencilStoreOp = storeOperationToVk(attDesc.storeOperatorStencil);
            vkAttachmentDesc.initialLayout = textureLayoutToVk(attDesc.initialLayout);
            vkAttachmentDesc.finalLayout = textureLayoutToVk(attDesc.finalLayout);
            vkAttachmentDescriptions[i] = vkAttachmentDesc;
            i++;
        }

        std::vector<VkSubpassDescription> vkSubpassDescriptions(getSubPassCount());

        std::vector<VkAttachmentReference> colorVkAttachmentRefs;
        std::vector<uint32> preserveVkAttachmentIndices;
        std::vector<VkAttachmentReference> depthStencilVkAttachmentRefs;

        uint32 colorVkAttachmentRefsOffset = 0;
        uint32 preserveVkAttachmentIndicesOffset = 0;
        uint32 depthStencilVkAttachmentRefsOffset = 0;

        i = 0;
        for (const SubPassDescription &subPassDesc : subPassDescs) {

            for (const auto &ref: subPassDesc.colorAttachmentsRefs) {
                VkAttachmentReference vkAttachmentRef = {};
                vkAttachmentRef.layout = textureLayoutToVk(ref.layout);
                vkAttachmentRef.attachment = ref.index;
                colorVkAttachmentRefs.push_back(vkAttachmentRef);
            }

            for (const auto &idx : subPassDesc.preserveAttachmentsIndices) {
                preserveVkAttachmentIndices.push_back(idx);
            }

            if (subPassDesc.depthStencilAttachmentsRef) {
                VkAttachmentReference vkAttachmentReference;
                vkAttachmentReference.layout = textureLayoutToVk(subPassDesc.depthStencilAttachmentsRef.value().layout);
                vkAttachmentReference.attachment = subPassDesc.depthStencilAttachmentsRef.value().index;
                depthStencilVkAttachmentRefs.push_back(vkAttachmentReference);
            }

            VkSubpassDescription vkSubpassDesc = {};
            vkSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            vkSubpassDesc.inputAttachmentCount = 0; //TODO: Not implemented
            vkSubpassDesc.pInputAttachments = nullptr;

            //The index of the attachment in this array is directly referenced from the fragment shader.
            vkSubpassDesc.colorAttachmentCount = static_cast<uint32_t>(subPassDesc.colorAttachmentsRefs.size());
            vkSubpassDesc.pColorAttachments = colorVkAttachmentRefs.data() + colorVkAttachmentRefsOffset;

            vkSubpassDesc.pResolveAttachments = nullptr;  //TODO: Not implemented

            vkSubpassDesc.pDepthStencilAttachment = hasDepthStencilAttachment() ? (depthStencilVkAttachmentRefs.data() + preserveVkAttachmentIndicesOffset) : nullptr;

            vkSubpassDesc.preserveAttachmentCount = static_cast<uint32_t>(subPassDesc.preserveAttachmentsIndices.size());
            vkSubpassDesc.pPreserveAttachments = preserveVkAttachmentIndices.data() + depthStencilVkAttachmentRefsOffset;

            vkSubpassDescriptions[i] = vkSubpassDesc;
            i++;

            colorVkAttachmentRefsOffset += vkSubpassDesc.colorAttachmentCount;
            preserveVkAttachmentIndicesOffset += vkSubpassDesc.preserveAttachmentCount;
            depthStencilVkAttachmentRefsOffset += vkSubpassDesc.pDepthStencilAttachment ? 1 : 0;
        }

        std::vector<VkSubpassDependency> vkSubpassDependencies(subPassDeps.size());
        i = 0;
        for (const auto &subPassDep : subPassDeps) {
            VkSubpassDependency vkSubPassDependency;
            vkSubPassDependency.srcSubpass = subPassDep.srcSubPassIndex;
            vkSubPassDependency.dstSubpass = subPassDep.dstSubPassIndex;
            vkSubPassDependency.srcStageMask = pipelineStageMaskToVk(subPassDep.srcStageMask);
            vkSubPassDependency.dstStageMask = pipelineStageMaskToVk(subPassDep.dstStageMask);
            vkSubPassDependency.srcAccessMask = accessMaskToVk(subPassDep.srcAccessMask);
            vkSubPassDependency.dstAccessMask = accessMaskToVk(subPassDep.dstAccessMask);
            vkSubPassDependency.dependencyFlags = dependencyMaskToVk(subPassDep.dependencyFlags);
            vkSubpassDependencies[i] = vkSubPassDependency;
            i++;
        }

        //Create render pass
        VkRenderPassCreateInfo vkRrenderPassInfo = {};
        vkRrenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        vkRrenderPassInfo.attachmentCount = getAttachmentCount();
        vkRrenderPassInfo.pAttachments = vkAttachmentDescriptions.data();
        vkRrenderPassInfo.subpassCount = getSubPassCount();
        vkRrenderPassInfo.pSubpasses = vkSubpassDescriptions.data();
        vkRrenderPassInfo.dependencyCount = static_cast<uint32>(subPassDeps.size());
        vkRrenderPassInfo.pDependencies = vkSubpassDependencies.data();

        BZ_ASSERT_VK(vkCreateRenderPass(getDevice(), &vkRrenderPassInfo, nullptr, forceClear ? &nativeHandle.forceClear : &nativeHandle.original));
    }
}