#pragma once

#include "Bhazel/Renderer/Framebuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"


namespace BZ {

    class VulkanContext;

    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(const std::vector<Ref<TextureView>> &textureViews);
        ~VulkanFramebuffer() override;

    private:
        VulkanContext &context;

        VkRenderPass renderPassHandle;
        VkFramebuffer framebufferHandle;

        void initRenderPass();

        friend class VulkanContext;
        friend class VulkanPipelineState;
    };
}