#pragma once

#include "Graphics/Framebuffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Platform/Vulkan/Internal/VulkanRenderPass.h"



namespace BZ {

    //On Vulkan this will manage a VkFramebuffer and a correspondent VkRenderPass with a single SubRenderPass.
    class VulkanFramebuffer : public Framebuffer, public VulkanGpuObject<VkFramebuffer> {
    public:
        explicit VulkanFramebuffer(const Builder &builder);
        ~VulkanFramebuffer() override;

        const VulkanRenderPass& getOriginalRenderPass() const { return originalrenderPass; }
        const VulkanRenderPass& getClearRenderPass() const { return clearRenderPass; }

    private:
        VulkanRenderPass originalrenderPass;
        VulkanRenderPass clearRenderPass;
    };
}