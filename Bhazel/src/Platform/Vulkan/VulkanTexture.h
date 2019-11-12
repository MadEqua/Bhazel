#pragma once

#include "Graphics/Texture.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    class VulkanTexture2D : public Texture2D, public VulkanGpuObject<VkImage> {
    public:
        static Ref<VulkanTexture2D> wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        VulkanTexture2D(const std::string &path, TextureFormat format);
        VulkanTexture2D(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format);
        
        //Coming from an already existent VkImage. Used on the swapchain images.
        VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat);

        ~VulkanTexture2D() override;

    private:
        bool isWrapping;

        VmaAllocation allocationHandle;

        VkBuffer stagingBufferHandle;
        VmaAllocation stagingBufferAllocationHandle;

        void init(const byte *data, uint32 dataSize, uint32 width, uint32 height);

        void initStagingBuffer(uint32 size);
        void destroyStagingBuffer();

        VkCommandBuffer beginSingleTimeCommands();
        void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage destImage, uint32 width, uint32 height);
        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    };


    class VulkanTextureView : public TextureView, public VulkanGpuObject<VkImageView> {
    public:
        explicit VulkanTextureView(const Ref<Texture> &texture);
        ~VulkanTextureView() override;
    };


    class VulkanSampler : public Sampler, public VulkanGpuObject<VkSampler> {
    public:
        VulkanSampler(const Builder &builder);
        ~VulkanSampler() override;
    };
}