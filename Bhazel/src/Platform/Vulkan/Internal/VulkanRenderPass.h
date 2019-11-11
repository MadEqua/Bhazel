#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Graphics/Framebuffer.h"

namespace BZ {

    class VulkanDevice;

    class VulkanRenderPass {
    public:
        VulkanRenderPass() = default;

        void init(VkDevice device, const std::vector<Framebuffer::Attachment> &colorAttachments, const std::optional<Framebuffer::Attachment> &depthStencilAttachment, bool forceClearAttachments);
        void destroy();

        VkRenderPass getNativeHandle() const { return nativeHandle; }

    private:
        VkRenderPass nativeHandle = VK_NULL_HANDLE;
        VkDevice device;
    };
}