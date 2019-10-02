#pragma once

#include "Bhazel/Renderer/Texture.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"


namespace BZ {

    class VulkanContext;

    //TODO to a good location. this is used in many places.
    VkFormat textureFormatToVk(TextureFormat format);


    class VulkanTexture2D : public Texture2D {
    public:
        static Ref<VulkanTexture2D> create(VkImage vkImage, uint32 width, uint32 height);

        explicit VulkanTexture2D(const std::string &path, TextureFormat format);
        
        //Coming from an already existent VkImage. Used on the swapchain images.
        explicit VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height);

        ~VulkanTexture2D() override;

    private:
        VulkanContext &context;
        VkImage imageHandle;

        friend class VulkanTextureView;
    };


    class VulkanTextureView : public TextureView {
    public:
        explicit VulkanTextureView(const Ref<Texture>& texture);
        virtual ~VulkanTextureView() = default;

    private:
        VulkanContext& context;
        VkImageView imageViewHandle;

        friend class VulkanFramebuffer;
    };
}