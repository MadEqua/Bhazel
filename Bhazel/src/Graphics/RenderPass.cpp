#include "bzpch.h"

#include "RenderPass.h"

#include "Core/Application.h"

#include "Graphics/GraphicsContext.h"


namespace BZ {

    RenderPass::RenderPass(const std::initializer_list<AttachmentDescription> &descs,
                           const std::initializer_list<SubPassDescription> &subPassDescs,
                           const std::initializer_list<SubPassDependency> &subPassDeps) :
        attachmentDescs(descs), 
        subPassDescs(subPassDescs),
        subPassDeps(subPassDeps) {
        BZ_ASSERT_CORE(descs.size() != 0, "Creating a RenderPass with no AttachmentDescriptions!");

        uint32 idx = 0;
        for (const auto &att : descs) {
            if (att.format.isColor()) {
                colorAttachmentIndices.push_back(idx);
            }
            else if (att.format.isDepth()) {
                BZ_ASSERT_CORE(!depthStencilAttachmentDescIndex, "Adding more than one DepthStencilAttachment!");
                depthStencilAttachmentDescIndex.emplace(idx);
            }
            idx++;
        }

        init(false);
        init(true);
    }

    RenderPass::~RenderPass() {
        vkDestroyRenderPass(BZ_GRAPHICS_DEVICE.getHandle(), handle.original, nullptr);
        vkDestroyRenderPass(BZ_GRAPHICS_DEVICE.getHandle(), handle.forceClear, nullptr);
    }

    Ref<RenderPass> RenderPass::create(const std::initializer_list<AttachmentDescription> &descs,
                                       const std::initializer_list<SubPassDescription> &subPassDescs,
                                       const std::initializer_list<SubPassDependency> &subPassDeps) {
        return MakeRef<RenderPass>(descs, subPassDescs, subPassDeps);
    }

    const AttachmentDescription& RenderPass::getColorAttachmentDescription(uint32 index) const {
        BZ_ASSERT_CORE(index < getColorAttachmentCount(), "Index {} is out of range!", index);
        return attachmentDescs[colorAttachmentIndices[index]];
    }

    const AttachmentDescription* RenderPass::getDepthStencilAttachmentDescription() const {
        if (depthStencilAttachmentDescIndex.has_value())
            return &attachmentDescs[depthStencilAttachmentDescIndex.value()];
        return nullptr;
    }

    void RenderPass::init(bool forceClear) {
        std::vector<VkAttachmentDescription> vkAttachmentDescriptions(getAttachmentCount());

        int i = 0;
        for (const auto &attDesc : attachmentDescs) {
            VkAttachmentDescription vkAttachmentDesc = {};
            vkAttachmentDesc.format = attDesc.format;
            vkAttachmentDesc.samples = attDesc.samples;
            vkAttachmentDesc.loadOp = forceClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : attDesc.loadOperatorColorAndDepth;
            vkAttachmentDesc.storeOp = attDesc.storeOperatorColorAndDepth;
            vkAttachmentDesc.stencilLoadOp = forceClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : attDesc.loadOperatorStencil;
            vkAttachmentDesc.stencilStoreOp = attDesc.storeOperatorStencil;
            vkAttachmentDesc.initialLayout = forceClear ? VK_IMAGE_LAYOUT_UNDEFINED : attDesc.initialLayout;
            vkAttachmentDesc.finalLayout = attDesc.finalLayout;
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

            for (const auto &ref : subPassDesc.colorAttachmentsRefs) {
                VkAttachmentReference vkAttachmentRef = {};
                vkAttachmentRef.layout = ref.layout;
                vkAttachmentRef.attachment = ref.index;
                colorVkAttachmentRefs.push_back(vkAttachmentRef);
            }

            for (const auto &idx : subPassDesc.preserveAttachmentsIndices) {
                preserveVkAttachmentIndices.push_back(idx);
            }

            if (subPassDesc.depthStencilAttachmentsRef) {
                VkAttachmentReference vkAttachmentReference;
                vkAttachmentReference.layout = subPassDesc.depthStencilAttachmentsRef.value().layout;
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
            vkSubPassDependency.srcStageMask = subPassDep.srcStageMask;
            vkSubPassDependency.dstStageMask = subPassDep.dstStageMask;
            vkSubPassDependency.srcAccessMask = subPassDep.srcAccessMask;
            vkSubPassDependency.dstAccessMask = subPassDep.dstAccessMask;
            vkSubPassDependency.dependencyFlags = subPassDep.dependencyFlags;
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

        BZ_ASSERT_VK(vkCreateRenderPass(BZ_GRAPHICS_DEVICE.getHandle(), &vkRrenderPassInfo, nullptr, forceClear ? &handle.forceClear : &handle.original));
    }
}