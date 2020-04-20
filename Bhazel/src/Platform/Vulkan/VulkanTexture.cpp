#include "bzpch.h"

#include "VulkanTexture.h"

#include "Core/Application.h"
#include "Core/Utils.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    static void generateMipmaps(const Texture &texture, VkImage vkImage, VkCommandBuffer commandBuffer) {
        auto &graphicsContext = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());

        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(graphicsContext.getDevice().getPhysicalDevice().getNativeHandle(),
            textureFormatToVk(texture.getFormat().format), &formatProperties);

        BZ_CRITICAL_ERROR(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
            "Linear interpolation for blitting is not supported. Cannot generate mipmaps!");

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vkImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayers();
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texture.getDimensions().x;
        int32_t mipHeight = texture.getDimensions().y;

        for (uint32_t i = 1; i < texture.getMipLevels(); i++) {
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
            blit.srcSubresource.layerCount = texture.getLayers();
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1?mipWidth / 2:1, mipHeight > 1?mipHeight / 2:1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = texture.getLayers();

            vkCmdBlitImage(commandBuffer,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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

        barrier.subresourceRange.baseMipLevel = texture.getMipLevels() - 1;
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

    static void initStagingBuffer(uint32 size, VkBuffer *vkBuffer, VmaAllocation *vmaAllocation) {
        auto &graphicsContext = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::CpuToGpu);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::CpuToGpu);
        BZ_ASSERT_VK(vmaCreateBuffer(graphicsContext.getMemoryAllocator(), &bufferInfo, &allocInfo, vkBuffer, vmaAllocation, nullptr));
    }

    static void destroyStagingBuffer(VkBuffer vkBuffer, VmaAllocation vmaAllocation) {
        auto &graphicsContext = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());
        vmaDestroyBuffer(graphicsContext.getMemoryAllocator(), vkBuffer, vmaAllocation);
    }

    static VkCommandBuffer beginCommandBuffer() {
        auto &graphicsContext = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());

        //Graphics queue (and not transfer) because of the layout transition operations.
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandPool = graphicsContext.getCurrentFrameCommandPool(QueueProperty::Graphics, false).getNativeHandle();
        commandBufferAllocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(graphicsContext.getDevice().getNativeHandle(), &commandBufferAllocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    static void copyBufferToImage(const Texture &texture, VkBuffer vkBuffer, VkImage vkImage, VkCommandBuffer commandBuffer, uint32 bufferOffset, uint32 width, uint32 height, uint32 mipLevel) {
        VkBufferImageCopy region = {};
        region.bufferOffset = bufferOffset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = texture.getLayers();
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, vkBuffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    static void transitionImageLayout(const Texture &texture, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture.getMipLevels();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture.getLayers();
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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

    static void submitCommandBuffer(VkCommandBuffer commandBuffer) {
        auto &graphicsContext = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto queueHandle = graphicsContext.getDevice().getQueueContainerExclusive().graphics.getNativeHandle();
        vkQueueSubmit(queueHandle, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queueHandle);

        VkCommandPool cmdPool = graphicsContext.getCurrentFrameCommandPool(QueueProperty::Graphics, false).getNativeHandle();
        vkFreeCommandBuffers(graphicsContext.getDevice().getNativeHandle(), cmdPool, 1, &commandBuffer);
    }


    Ref<VulkanTexture2D> VulkanTexture2D::wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) {
        return MakeRef<VulkanTexture2D>(vkImage, width, height, vkFormat);
    }

    VulkanTexture2D::VulkanTexture2D(const char *path, TextureFormat format, MipmapData mipmapData) :
        Texture2D(format), isWrapping(false) {

        int desiredChannels = this->format.getChannelCount();

        VkCommandBuffer commBuffer = beginCommandBuffer();
        byte *stagingPtr;

        if (mipmapData.option == MipmapData::Options::Load) {
            mipLevels = mipmapData.mipLevels;

            std::vector<FileData> fileDatas(mipLevels);
            uint32 totalSize = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                std::string mipName = "_" + std::to_string(mipIdx);
                std::string fullPath = Utils::appendToFileName(path, mipName);
                
                const FileData fileData = loadFile(fullPath.c_str(), desiredChannels, true);
                fileDatas[mipIdx] = fileData;
                totalSize += fileData.width * fileData.height * desiredChannels;
            }

            dimensions.x = fileDatas[0].width;
            dimensions.y = fileDatas[0].height;

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = fileDatas[mipIdx].width * fileDatas[mipIdx].height * desiredChannels;
                memcpy(stagingPtr + stagingOffset, fileDatas[mipIdx].data, dataSize);
                freeData(fileDatas[mipIdx]);
                copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, stagingOffset, fileDatas[mipIdx].width, fileDatas[mipIdx].height, mipIdx);
                stagingOffset += dataSize;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
        else {
            const FileData fileData = loadFile(path, desiredChannels, true);

            dimensions.x = fileData.width;
            dimensions.y = fileData.height;
            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 dataSize = dimensions.x * dimensions.y * desiredChannels;
            initStagingBuffer(dataSize, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));
            memcpy(stagingPtr, fileData.data, dataSize);
            freeData(fileData);
            copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, 0, fileData.width, fileData.height, 0);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, nativeHandle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
    }

    VulkanTexture2D::VulkanTexture2D(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData) :
        Texture2D(format), isWrapping(false) {

        int channels = this->format.getChannelCount();

        VkCommandBuffer commBuffer = beginCommandBuffer();
        byte *stagingPtr;

        dimensions.x = width;
        dimensions.y = height;

        if (mipmapData.option == MipmapData::Options::Load) {
            mipLevels = mipmapData.mipLevels;

            std::vector<FileData> datas(mipLevels);
            uint32 totalSize = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                datas[mipIdx].data = data + totalSize;
                datas[mipIdx].width = dimensions.x >> mipIdx;
                datas[mipIdx].height = dimensions.y >> mipIdx;
                totalSize += datas[mipIdx].width * datas[mipIdx].height * channels;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = datas[mipIdx].width * datas[mipIdx].height * channels;
                memcpy(stagingPtr + stagingOffset, datas[mipIdx].data, dataSize);
                copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, stagingOffset, datas[mipIdx].width, datas[mipIdx].height, mipIdx);
                stagingOffset += dataSize;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
        else {
            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 dataSize = dimensions.x * dimensions.y * channels;
            initStagingBuffer(dataSize, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));
            memcpy(stagingPtr, data, dataSize);
            copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, 0, dimensions.x, dimensions.y, 0);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, nativeHandle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
    }

    VulkanTexture2D::VulkanTexture2D(uint32 width, uint32 height, TextureFormat format):
        Texture2D(format), isWrapping(false) {

        dimensions.x = width;
        dimensions.y = height;
        mipLevels = 1;
        createImage(false, MipmapData::Options::DoNothing);
    }

    VulkanTexture2D::VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) :
        Texture2D(vkFormatToTextureFormat(vkFormat)), isWrapping(true) {

        BZ_ASSERT_CORE(vkImage != VK_NULL_HANDLE, "Invalid VkImage!");
        nativeHandle.imageHandle = vkImage;

        dimensions.x = width;
        dimensions.y = height;
        mipLevels = 1;
    }

    VulkanTexture2D::~VulkanTexture2D() {
        if(!isWrapping)
            vmaDestroyImage(getGraphicsContext().getMemoryAllocator(), nativeHandle.imageHandle, nativeHandle.allocationHandle);
    }

    void VulkanTexture2D::createImage(bool hasData, MipmapData mipmapData) {
        VkFormat vkFormat = textureFormatToVk(format.format);

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(dimensions.x);
        imageInfo.extent.height = static_cast<uint32_t>(dimensions.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vkFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        if (hasData) {
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if (mipmapData.option == MipmapData::Options::Generate) {
                imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
        }
        else {
            imageInfo.usage = format.isColor() ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (!isWrapping) {
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::GpuOnly);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::GpuOnly);
        BZ_ASSERT_VK(vmaCreateImage(getGraphicsContext().getMemoryAllocator(), &imageInfo, &allocInfo, &nativeHandle.imageHandle, &nativeHandle.allocationHandle, nullptr));
    }


    VulkanTextureCube::VulkanTextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) :
        TextureCube(format) {

        layers = 6;

        int desiredChannels = this->format.getChannelCount();

        VkCommandBuffer commBuffer = beginCommandBuffer();
        byte *stagingPtr;

        if (mipmapData.option == MipmapData::Options::Load) {
            mipLevels = mipmapData.mipLevels;

            //Mip0 (6 faces), Mip1 (6 faces), etc...
            std::vector<FileData> fileDatas(mipLevels * 6);
            uint32 totalSize = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                std::string mipName = "_" + std::to_string(mipIdx);
                
                for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                    std::string fullPath = basePath + Utils::appendToFileName(fileNames[faceIdx], mipName);
                    const FileData fileData = loadFile(fullPath.c_str(), desiredChannels, false);
                    fileDatas[mipIdx * 6 + faceIdx] = fileData;
                    totalSize += fileData.width * fileData.height * desiredChannels;
                }
            }

            dimensions.x = fileDatas[0].width;
            dimensions.y = fileDatas[0].height;

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 faceOffset = 0;
            uint32 copyOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 faceSize = fileDatas[mipIdx * 6].width * fileDatas[mipIdx * 6].height * desiredChannels;

                for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                    uint32 fileDatasIdx = mipIdx * 6 + faceIdx;
                    memcpy(stagingPtr + faceOffset, fileDatas[fileDatasIdx].data, faceSize);
                    freeData(fileDatas[fileDatasIdx]);
                    faceOffset += faceSize;
                }

                copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, copyOffset, fileDatas[mipIdx * 6].width, fileDatas[mipIdx * 6].height, mipIdx);
                copyOffset = faceOffset;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
        else {
            std::vector<FileData> fileDatas(6);
            for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                std::string fullPath = std::string(basePath) + fileNames[faceIdx];
                fileDatas[faceIdx] = loadFile(fullPath.c_str(), desiredChannels, false);
            }

            dimensions.x = fileDatas[0].width;
            dimensions.y = fileDatas[0].height;

            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 faceDataSize = dimensions.x * dimensions.y * desiredChannels;
            initStagingBuffer(faceDataSize * 6, &nativeHandle.stagingBufferHandle, &nativeHandle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                memcpy(stagingPtr + stagingOffset, fileDatas[faceIdx].data, faceDataSize);
                freeData(fileDatas[faceIdx]);
                stagingOffset += faceDataSize;
            }
            copyBufferToImage(*this, nativeHandle.stagingBufferHandle, nativeHandle.imageHandle, commBuffer, 0, dimensions.x, dimensions.y, 0);

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), nativeHandle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, nativeHandle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, nativeHandle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(nativeHandle.stagingBufferHandle, nativeHandle.stagingBufferAllocationHandle);
        }
    }

    VulkanTextureCube::~VulkanTextureCube() {
        vmaDestroyImage(getGraphicsContext().getMemoryAllocator(), nativeHandle.imageHandle, nativeHandle.allocationHandle);
    }

    void VulkanTextureCube::createImage(bool hasData, MipmapData mipmapData) {
        VkFormat vkFormat = textureFormatToVk(format.format);

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(dimensions.x);
        imageInfo.extent.height = static_cast<uint32_t>(dimensions.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 6;
        imageInfo.format = vkFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (hasData) {
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if (mipmapData.option == MipmapData::Options::Generate) {
                imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
        }
        else {
            imageInfo.usage = format.isColor() ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::GpuOnly);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::GpuOnly);
        BZ_ASSERT_VK(vmaCreateImage(getGraphicsContext().getMemoryAllocator(), &imageInfo, &allocInfo, &nativeHandle.imageHandle, &nativeHandle.allocationHandle, nullptr));
    }


    VulkanTextureView::VulkanTextureView(const Ref<Texture2D> &texture2D) :
        TextureView(texture2D) {
        init(VK_IMAGE_VIEW_TYPE_2D, static_cast<VulkanTexture2D &>(*texture2D).getNativeHandle().imageHandle);
    }

    VulkanTextureView::VulkanTextureView(const Ref<TextureCube> &textureCube) :
        TextureView(textureCube) {
        init(VK_IMAGE_VIEW_TYPE_CUBE, static_cast<VulkanTextureCube &>(*textureCube).getNativeHandle().imageHandle);
    }

    VulkanTextureView::~VulkanTextureView() {
        vkDestroyImageView(getDevice(), nativeHandle, nullptr);
    }

    void VulkanTextureView::init(VkImageViewType viewType, VkImage vkImage) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkImage;
        imageViewCreateInfo.viewType = viewType;
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
        //if (texture->getFormat().isStencil()) {
        //    imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        //}

        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = texture->getMipLevels();
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = texture->getLayers();
        BZ_ASSERT_VK(vkCreateImageView(getDevice(), &imageViewCreateInfo, nullptr, &nativeHandle));
    }



    VulkanSampler::VulkanSampler(const Builder &builder) {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = filterModeToVk(builder.minFilter);
        samplerInfo.magFilter = filterModeToVk(builder.magFilter);
        samplerInfo.mipmapMode = sampleMipmapModeToVk(builder.mipmapFilter);
        samplerInfo.addressModeU = addressModeToVk(builder.addressModeU);
        samplerInfo.addressModeV = addressModeToVk(builder.addressModeV);
        samplerInfo.addressModeW = addressModeToVk(builder.addressModeW);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE; //TODO
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.compareEnable = builder.compareEnabled? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = compareFunctionToVk(builder.compareFunction);
        samplerInfo.minLod = static_cast<float>(builder.minMipmap);
        samplerInfo.maxLod = static_cast<float>(builder.maxMipmap);
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = builder.unnormalizedCoordinate ? VK_TRUE : VK_FALSE;

        BZ_ASSERT_VK(vkCreateSampler(getDevice(), &samplerInfo, nullptr, &nativeHandle));
    }

    VulkanSampler::~VulkanSampler() {
        vkDestroySampler(getDevice(), nativeHandle, nullptr);
    }
}