#pragma once

#include "Graphics/Texture.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    struct VulkanTextureData {
        VkImage imageHandle;
        VmaAllocation allocationHandle;

        VkBuffer stagingBufferHandle;
        VmaAllocation stagingBufferAllocationHandle;
    };


    class VulkanTexture2D : public Texture2D, public VulkanGpuObject<VulkanTextureData> {
    public:
        static Ref<VulkanTexture2D> wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        VulkanTexture2D(const char *path, TextureFormat format, MipmapData mipmapData);
        VulkanTexture2D(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData);
        VulkanTexture2D(uint32 width, uint32 height, TextureFormat format);
        
        //Coming from an already existent VkImage. Used on the swapchain images.
        VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        ~VulkanTexture2D() override;

    private:
        void createImage(bool hasData, MipmapData mipmapData);
        bool isWrapping;
    };


    class VulkanTextureCube : public TextureCube, public VulkanGpuObject<VulkanTextureData> {
    public:
        VulkanTextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData);

        ~VulkanTextureCube() override;

    private:
        void createImage(bool hasData, MipmapData mipmapData);
    };


    class VulkanTextureView : public TextureView, public VulkanGpuObject<VkImageView> {
    public:
        explicit VulkanTextureView(const Ref<Texture2D> &texture);
        explicit VulkanTextureView(const Ref<TextureCube> &textureCube);

        ~VulkanTextureView() override;

    private:
        void init(VkImageViewType viewType, VkImage vkImage);
    };


    class VulkanSampler : public Sampler, public VulkanGpuObject<VkSampler> {
    public:
        VulkanSampler(const Builder &builder);
        ~VulkanSampler() override;
    };
}