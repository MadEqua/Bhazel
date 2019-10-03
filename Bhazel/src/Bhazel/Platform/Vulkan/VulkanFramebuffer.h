#pragma once

#include "Bhazel/Renderer/Framebuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    struct VulkanFramebufferHandles {
        VkRenderPass renderPassHandle;
        VkFramebuffer frameBufferHandle;
    };

    class VulkanFramebuffer : public Framebuffer, public VulkanGpuObject<VulkanFramebufferHandles> {
    public:
        VulkanFramebuffer(const std::vector<Ref<TextureView>> &textureViews);
        ~VulkanFramebuffer() override;

    private:
        void initRenderPass();
    };
}