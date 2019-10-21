#pragma once

#include "Graphics/Framebuffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    struct VulkanFramebufferHandles {
        VkRenderPass renderPassHandle;
        VkFramebuffer frameBufferHandle;
    };

    //On Vulkan this will manage a VkFramebuffer and a correspondent VkRenderPass with a single SubRenderPass.
    class VulkanFramebuffer : public Framebuffer, public VulkanGpuObject<VulkanFramebufferHandles> {
    public:
        explicit VulkanFramebuffer(const Builder &builder);
        ~VulkanFramebuffer() override;

    private:
        void initRenderPass();
    };
}