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
        VulkanRenderPass(const std::initializer_list<AttachmentDescription> &descs,
                         const std::initializer_list<SubPassDescription> &subPassDescs,
                         const std::initializer_list<SubPassDependency> &subPassDeps);
        ~VulkanRenderPass() override;

    private:
        void init(bool forceClear);
    };
}