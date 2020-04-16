#pragma once

#include "Graphics/Framebuffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    class RenderPass;
    class TextureView;

    class VulkanFramebuffer : public Framebuffer, public VulkanGpuObject<VkFramebuffer> {
    public:
        explicit VulkanFramebuffer(const Ref<RenderPass> &renderPass, const std::initializer_list<Ref<TextureView>> &textureViews, const glm::ivec3 &dimensions);
        ~VulkanFramebuffer() override;
    };
}