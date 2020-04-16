#include "bzpch.h"

#include "RenderPass.h"
#include "Graphics.h"

#include "Platform/Vulkan/VulkanRenderPass.h"


namespace BZ {

    RenderPass::RenderPass(const std::initializer_list<AttachmentDescription> &descs) {
        BZ_ASSERT_CORE(descs.size() != 0, "Creating a RenderPass with no AttachmentDescriptions!");

        for (const auto &att : descs) {
            if (att.format.isColor()) {
                colorAttachmentDescs.push_back(att);
            }
            else if (att.format.isDepth()) {
                BZ_ASSERT_CORE(!depthStencilAttachmentDesc, "Adding more than one DepthStencilAttachment!");
                depthStencilAttachmentDesc.emplace(att);
            }
        }
    }

    Ref<RenderPass> RenderPass::create(const std::initializer_list<AttachmentDescription> &descs) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanRenderPass>(descs);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    const std::vector<AttachmentDescription>& RenderPass::getColorAttachmentDescriptions() const {
        return colorAttachmentDescs;
    }

    const AttachmentDescription& RenderPass::getColorAttachmentDescription(uint32 index) const {
        BZ_ASSERT_CORE(index < getColorAttachmentCount(), "Index {} is out of range!", index);
        return colorAttachmentDescs[index];
    }

    const AttachmentDescription* RenderPass::getDepthStencilAttachmentDescription() const {
        if (depthStencilAttachmentDesc.has_value())
            return &depthStencilAttachmentDesc.value();
        return nullptr;
    }
}