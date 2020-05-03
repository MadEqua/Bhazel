#include "bzpch.h"

#include "RenderPass.h"
#include "Graphics.h"

#include "Platform/Vulkan/VulkanRenderPass.h"


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
    }

    Ref<RenderPass> RenderPass::create(const std::initializer_list<AttachmentDescription> &descs,
                                       const std::initializer_list<SubPassDescription> &subPassDescs,
                                       const std::initializer_list<SubPassDependency> &subPassDeps) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanRenderPass>(descs, subPassDescs, subPassDeps);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
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
}