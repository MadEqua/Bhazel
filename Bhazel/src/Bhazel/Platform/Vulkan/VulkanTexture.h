#pragma once

#include "Bhazel/Renderer/Texture.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    class VulkanTexture2D : public Texture2D, public VulkanGpuObject<VkImage> {
    public:
        static Ref<VulkanTexture2D> wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        VulkanTexture2D(const std::string &path, TextureFormat format);
        
        //Coming from an already existent VkImage. Used on the swapchain images.
        VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        ~VulkanTexture2D() override;

    private:
        bool isWrapping;
    };


    class VulkanTextureView : public TextureView, public VulkanGpuObject<VkImageView> {
    public:
        explicit VulkanTextureView(const Ref<Texture>& texture);
        ~VulkanTextureView() override;
    };
}