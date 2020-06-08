#pragma once

#include "Graphics/GpuObject.h"
#include "Graphics/Internal/VulkanIncludes.h"

#include "Graphics/Texture.h"


namespace BZ {

struct SubPassDependency {
    // Indices refer to parent RenderPass SubPass list, or VK_SUBPASS_EXTERNAL.
    int srcSubPassIndex;
    int dstSubPassIndex;

    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;

    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    VkDependencyFlags dependencyFlags;
};


/*-------------------------------------------------------------------------------------------*/
struct AttachmentReference {
    // Index refers to parent RenderPass AttachmentDescription list.
    uint32 index;

    // Layout the attachment uses during the SubPass.
    VkImageLayout layout;
};

struct SubPassDescription {

    // TODO: not implemented
    // std::vector<uint32> inputAttachmentsRefs;
    // std::vector<uint32> resolveAttachmentsRefs;

    // Order in this list correspond to fragment shader outputs.
    std::vector<AttachmentReference> colorAttachmentsRefs;
    std::optional<AttachmentReference> depthStencilAttachmentsRef;
    std::vector<uint32> preserveAttachmentsIndices;
};


/*-------------------------------------------------------------------------------------------*/
struct AttachmentDescription {
    TextureFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp loadOperatorColorAndDepth;
    VkAttachmentStoreOp storeOperatorColorAndDepth;
    VkAttachmentLoadOp loadOperatorStencil;
    VkAttachmentStoreOp storeOperatorStencil;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
    VkClearValue clearValue = {}; // RGBA or Depth/Stencil.
};

class RenderPass : public GpuObject<VkRenderPass> {
  public:
    static Ref<RenderPass> create(const std::initializer_list<AttachmentDescription> &descs,
                                  const std::initializer_list<SubPassDescription> &subPassDescs,
                                  const std::initializer_list<SubPassDependency> &subPassDeps = {});

    uint32 getAttachmentCount() const { return static_cast<uint32>(attachmentDescs.size()); }
    uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorAttachmentIndices.size()); }
    bool hasDepthStencilAttachment() const { return depthStencilAttachmentDescIndex.has_value(); }

    uint32 getSubPassCount() const { return static_cast<uint32>(subPassDescs.size()); }

    const AttachmentDescription &getAttachmentDescription(uint32 index) const;
    const AttachmentDescription &getColorAttachmentDescription(uint32 index) const;
    const AttachmentDescription *getDepthStencilAttachmentDescription() const;

    const SubPassDescription &getSubPassDescription(uint32 index) const;
    const SubPassDependency &getSubPassDependency(uint32 index) const;

    const std::vector<AttachmentDescription> &getAttachmentDescriptions() const { return attachmentDescs; }
    const std::vector<SubPassDescription> &getSubPassDescriptions() const { return subPassDescs; }
    const std::vector<SubPassDependency> &getSubPassDependencies() const { return subPassDeps; }

    RenderPass(const std::initializer_list<AttachmentDescription> &descs,
               const std::initializer_list<SubPassDescription> &subPassDescs,
               const std::initializer_list<SubPassDependency> &subPassDeps);
    ~RenderPass();

    BZ_NON_COPYABLE(RenderPass);

  private:
    void init();

    std::vector<AttachmentDescription> attachmentDescs;
    std::vector<SubPassDescription> subPassDescs;
    std::vector<SubPassDependency> subPassDeps;

    // Indices refer to the main AttachmentDescription list.
    std::vector<uint32> colorAttachmentIndices;

    // Can be Depth only or DepthStencil
    std::optional<uint32> depthStencilAttachmentDescIndex;
};
}