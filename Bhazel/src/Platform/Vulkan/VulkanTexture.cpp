#include "bzpch.h"

#include "VulkanTexture.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"

#include <stb_image.h>


namespace BZ {

    Ref<VulkanTexture2D> VulkanTexture2D::wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) {
        return MakeRef<VulkanTexture2D>(vkImage, width, height, vkFormat);
    }

    VulkanTexture2D::VulkanTexture2D(const char* path, TextureFormat format, bool generateMipmaps) :
        Texture2D(format), isWrapping(false) {

        int width, height;
        const byte *data = loadFile(path, true, width, height);

        init(data, width * height * 4, width, height, generateMipmaps);
        freeData(data);
    }

    VulkanTexture2D::VulkanTexture2D(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format, bool generateMipmaps) :
        Texture2D(format), isWrapping(false) {
        init(data, dataSize, width, height, generateMipmaps);
    }

    VulkanTexture2D::VulkanTexture2D(uint32 width, uint32 height, TextureFormat format):
        Texture2D(format), isWrapping(false) {
        init(nullptr, 0, width, height, false);
    }

    VulkanTexture2D::VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) :
        Texture2D(vkFormatToTextureFormat(vkFormat)), isWrapping(true) {

        BZ_ASSERT_CORE(vkImage != VK_NULL_HANDLE, "Invalid VkImage!");
        nativeHandle = vkImage;

        dimensions.x = width;
        dimensions.y = height;

        mipLevels = 1;
    }

    VulkanTexture2D::~VulkanTexture2D() {
        if(!isWrapping)
            vmaDestroyImage(getGraphicsContext().getMemoryAllocator(), nativeHandle, allocationHandle);
    }

    void VulkanTexture2D::init(const byte *data, uint32 dataSize, uint32 width, uint32 height, bool generateMipmaps) {
        dimensions.x = width;
        dimensions.y = height;

        if (generateMipmaps)
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        else
            mipLevels = 1;

        VkFormat vkFormat = textureFormatToVk(format.format);

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vkFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (data == nullptr) {
            imageInfo.usage = format.isColor() ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else {
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            if (generateMipmaps) {
                imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
        }

        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::GpuOnly);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::GpuOnly);
        BZ_ASSERT_VK(vmaCreateImage(getGraphicsContext().getMemoryAllocator(), &imageInfo, &allocInfo, &nativeHandle, &allocationHandle, nullptr));

        if (data != nullptr) {
            initStagingBuffer(dataSize);

            void *ptr;
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), stagingBufferAllocationHandle, &ptr));
            memcpy(ptr, data, dataSize);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), stagingBufferAllocationHandle);

            //Transfer from staging buffer to device local image.
            VkCommandBuffer commBuffer = beginSingleTimeCommands();
            transitionImageLayout(commBuffer, nativeHandle, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(commBuffer, stagingBufferHandle, nativeHandle, width, height);

            if (generateMipmaps)
                this->generateMipmaps(commBuffer);
            else
                transitionImageLayout(commBuffer, nativeHandle, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            endSingleTimeCommands(commBuffer);

            destroyStagingBuffer();
        }
    }

    void VulkanTexture2D::generateMipmaps(VkCommandBuffer commandBuffer) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(getGraphicsContext().getDevice().getPhysicalDevice().getNativeHandle(), 
                                            textureFormatToVk(format.format), &formatProperties);

        BZ_CRITICAL_ERROR(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
            "Linear interpolation for blitting is not supported. Cannot generate mipmaps!");

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = nativeHandle;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = dimensions.x;
        int32_t mipHeight = dimensions.y;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                nativeHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                nativeHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void VulkanTexture2D::initStagingBuffer(uint32 size) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::CpuToGpu);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::CpuToGpu);
        BZ_ASSERT_VK(vmaCreateBuffer(getGraphicsContext().getMemoryAllocator(), &bufferInfo, &allocInfo, &stagingBufferHandle, &stagingBufferAllocationHandle, nullptr));
    }

    void VulkanTexture2D::destroyStagingBuffer() {
        vmaDestroyBuffer(getGraphicsContext().getMemoryAllocator(), stagingBufferHandle, stagingBufferAllocationHandle);
    }

    VkCommandBuffer VulkanTexture2D::beginSingleTimeCommands() {

        //Graphics queue (and not transfer) because of the layout transition operations.
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandPool = getGraphicsContext().getCurrentFrameCommandPool(QueueProperty::Graphics, false).getNativeHandle();
        commandBufferAllocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(getDevice(), &commandBufferAllocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void VulkanTexture2D::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32 width, uint32 height) {
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, stagingBufferHandle, this->nativeHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanTexture2D::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            BZ_ASSERT_ALWAYS_CORE("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void VulkanTexture2D::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto queueHandle = getGraphicsContext().getDevice().getQueueContainerExclusive().graphics.getNativeHandle();
        vkQueueSubmit(queueHandle, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queueHandle);

        VkCommandPool cmdPool = getGraphicsContext().getCurrentFrameCommandPool(QueueProperty::Graphics, false).getNativeHandle();
        vkFreeCommandBuffers(getDevice(), cmdPool, 1, &commandBuffer);
    }


    VulkanTextureView::VulkanTextureView(const Ref<Texture> &texture) :
        TextureView(texture) {

        //TODO: fill correctly
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = static_cast<VulkanTexture2D &>(*texture).getNativeHandle();
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = textureFormatToVk(texture->getFormat().format);
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        if (texture->getFormat().isColor()) {
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        else if (texture->getFormat().isDepth()) {
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (texture->getFormat().isStencil()) {
            imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = texture->getMipLevels();
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        BZ_ASSERT_VK(vkCreateImageView(getDevice(), &imageViewCreateInfo, nullptr, &nativeHandle));
    }

    VulkanTextureView::~VulkanTextureView() {
        vkDestroyImageView(getDevice(), nativeHandle, nullptr);
    }


    VulkanSampler::VulkanSampler(const Builder &builder) {

        //TODO: some fields
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = filterModeToVk(builder.minFilter);
        samplerInfo.magFilter = filterModeToVk(builder.magFilter);
        samplerInfo.addressModeU = addressModeToVk(builder.addressModeU);
        samplerInfo.addressModeV = addressModeToVk(builder.addressModeV);
        samplerInfo.addressModeW = addressModeToVk(builder.addressModeW);
        samplerInfo.anisotropyEnable = 0; 
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = builder.unnormalizedCoordinate ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = sampleMipmapModeToVk(builder.mipmapFilter);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = static_cast<float>(builder.minMipmap);
        samplerInfo.maxLod = static_cast<float>(builder.maxMipmap);

        BZ_ASSERT_VK(vkCreateSampler(getDevice(), &samplerInfo, nullptr, &nativeHandle));
    }

    VulkanSampler::~VulkanSampler() {
        vkDestroySampler(getDevice(), nativeHandle, nullptr);
    }
}