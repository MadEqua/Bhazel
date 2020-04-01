#pragma once

#include "Graphics/Texture.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    struct VulkanTextureData : public VulkanGpuObject<VkImage> {
        VmaAllocation allocationHandle;
        VkBuffer stagingBufferHandle;
        VmaAllocation stagingBufferAllocationHandle;
    };


    class VulkanTexture2D : public Texture2D {
    public:
        static Ref<VulkanTexture2D> wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        VulkanTexture2D(const char* path, TextureFormat format, bool generateMipmaps);
        VulkanTexture2D(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format, bool generateMipmaps);
        VulkanTexture2D(uint32 width, uint32 height, TextureFormat format);
        
        //Coming from an already existent VkImage. Used on the swapchain images.
        VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        ~VulkanTexture2D() override;

        const VulkanTextureData& getVulkanTextureData() const { return textureData; }

    private:
        void init(const byte *data, uint32 dataSize, uint32 width, uint32 height, bool generateMipmaps);

        VulkanTextureData textureData;
        bool isWrapping;
    };


    class VulkanTextureCube : public TextureCube {
    public:
        VulkanTextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, bool generateMipmaps);
        ~VulkanTextureCube() override;

        const VulkanTextureData& getVulkanTextureData() const { return textureData; }

    private:
        void init(const byte *data[6], uint32 faceDataSize, uint32 width, uint32 height, bool generateMipmaps);

        VulkanTextureData textureData;
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