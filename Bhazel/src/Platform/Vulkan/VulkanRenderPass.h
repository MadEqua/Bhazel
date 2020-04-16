#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"

#include "Graphics/RenderPass.h"


namespace BZ {

    struct RenderPassData {
        VkRenderPass original;
        VkRenderPass forceClear;
    };

    class VulkanRenderPass : public RenderPass, public VulkanGpuObject<RenderPassData> {
    public:
        VulkanRenderPass(const std::initializer_list<AttachmentDescription> &descs);
        ~VulkanRenderPass() override;

    private:
        void init(const std::initializer_list<AttachmentDescription> &descs, bool forceClear);
    };
}