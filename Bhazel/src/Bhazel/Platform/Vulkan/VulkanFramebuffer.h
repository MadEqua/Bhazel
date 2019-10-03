#pragma once

#include "Bhazel/Renderer/Framebuffer.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    struct VulkanFramebufferHandles {
        VkRenderPass renderPassHandle;
        VkFramebuffer frameBufferHandle;
    };

    //On Vulkan this will manage a VkFramebuffer and a correspondent VkRenderPass with a single SubRenderPass.
    class VulkanFramebuffer : public Framebuffer, public VulkanGpuObject<VulkanFramebufferHandles> {
    public:
        VulkanFramebuffer(const std::vector<Attachment> &attachments, const glm::ivec3 &dimensions);
        ~VulkanFramebuffer() override;

    private:
        void initRenderPass();
    };
}