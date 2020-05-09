#include "bzpch.h"

#include "Texture.h"

#include "Core/Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace BZ {

    static void generateMipmaps(const Texture &texture, VkImage vkImage, VkCommandBuffer commandBuffer) {
        auto &graphicsContext = Application::get().getGraphicsContext();

        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(graphicsContext.getDevice().getPhysicalDevice().getHandle(),
            texture.getFormat(), &formatProperties);

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
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
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
        auto &graphicsContext = Application::get().getGraphicsContext();

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        BZ_ASSERT_VK(vmaCreateBuffer(graphicsContext.getMemoryAllocator(), &bufferInfo, &allocInfo, vkBuffer, vmaAllocation, nullptr));
    }

    static void destroyStagingBuffer(VkBuffer vkBuffer, VmaAllocation vmaAllocation) {
        auto &graphicsContext = Application::get().getGraphicsContext();
        vmaDestroyBuffer(graphicsContext.getMemoryAllocator(), vkBuffer, vmaAllocation);
    }

    static VkCommandBuffer beginCommandBuffer() {
        auto &graphicsContext = Application::get().getGraphicsContext();

        //Graphics queue (and not transfer) because of the layout transition operations.
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandPool = graphicsContext.getCurrentFrameCommandPool(QueueProperty::Graphics, false).getHandle();
        commandBufferAllocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(graphicsContext.getDevice().getHandle(), &commandBufferAllocInfo, &commandBuffer);

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
        auto &graphicsContext = Application::get().getGraphicsContext();

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto queueHandle = graphicsContext.getDevice().getQueueContainerExclusive().graphics().getHandle();
        vkQueueSubmit(queueHandle, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queueHandle);

        VkCommandPool cmdPool = graphicsContext.getCurrentFrameCommandPool(QueueProperty::Graphics, false).getHandle();
        vkFreeCommandBuffers(graphicsContext.getDevice().getHandle(), cmdPool, 1, &commandBuffer);
    }


    /*-------------------------------------------------------------------------------------------*/
    TextureFormat::TextureFormat(VkFormat format) :
        format(format)  {
    }

    bool TextureFormat::isColor() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return true;
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return false;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isDepth() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return false;
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isStencil() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return false;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isDepthStencil() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return false;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isDepthOnly() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return false;
            case VK_FORMAT_D32_SFLOAT:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isSRGB() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return false;
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    bool TextureFormat::isFloatingPoint() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return false;
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return true;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return false;
        };
    }

    int TextureFormat::getChannelCount() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return 1;
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return 2;
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
                return 3;
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 4;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return 0;
        };
    }

    int TextureFormat::getSizePerChannel() const {
        switch(format) {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
                return 1;
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return 2;
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT:
                return 4;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
                BZ_ASSERT_ALWAYS_CORE("No uniform size per channel!");
                return 0;
            default:
                BZ_ASSERT_ALWAYS_CORE("Undefined TextureFormat!");
                return 0;
        };
    }

    int TextureFormat::getSizePerTexel() const {
        return getChannelCount() * getSizePerChannel();
    }


    /*-------------------------------------------------------------------------------------------*/
    Texture::Texture(TextureFormat format) :
        format(format) {
    }

    Texture::~Texture() {
        if (!isWrapping)
            vmaDestroyImage(getGraphicsContext().getMemoryAllocator(), handle.imageHandle, handle.allocationHandle);
    }

    Texture::FileData Texture::loadFile(const char* path, int desiredChannels, bool flip, float isFloatingPoint) {
        stbi_set_flip_vertically_on_load(flip);
        int channelsInFile, width, height;

        stbi_uc* data = nullptr;
        if (isFloatingPoint) {
            data = reinterpret_cast<stbi_uc*>(stbi_loadf(path, &width, &height, &channelsInFile, desiredChannels));
        }
        else {
            data = stbi_load(path, &width, &height, &channelsInFile, desiredChannels);
        }

        BZ_CRITICAL_ERROR_CORE(data, "Failed to load image '{}'. Reason: {}.", path, stbi_failure_reason());

        FileData ret;
        ret.data = static_cast<byte*>(data);
        ret.width = width;
        ret.height = height;
        return ret;
    }

    void Texture::freeData(const FileData &fileData) {
        stbi_image_free((void*)fileData.data);
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<Texture2D> Texture2D::create(const char *path, TextureFormat format, MipmapData mipmapData) {
        auto &assetsPath = Application::get().getAssetsPath();
        return MakeRef<Texture2D>((assetsPath + path).c_str(), format, mipmapData);
    }

    Ref<Texture2D> Texture2D::create(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData) {
        return MakeRef<Texture2D>(data, width, height, format, mipmapData);
    }

    Ref<Texture2D> Texture2D::createRenderTarget(uint32 width, uint32 height, uint32 layers, TextureFormat format) {
        return MakeRef<Texture2D>(width, height, layers, format);
    }

    Ref<Texture2D> Texture2D::wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) {
        return MakeRef<Texture2D>(new Texture2D(vkImage, width, height, vkFormat));
    }

    Texture2D::Texture2D(const char *path, TextureFormat format, MipmapData mipmapData) :
        Texture(format) {

        VkCommandBuffer commBuffer = beginCommandBuffer();
        byte *stagingPtr;

        if (mipmapData.option == MipmapData::Options::Load) {
            mipLevels = mipmapData.mipLevels;

            std::vector<FileData> fileDatas(mipLevels);
            uint32 totalSize = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                std::string mipName = "_" + std::to_string(mipIdx);
                std::string fullPath = Utils::appendToFileName(path, mipName);

                const FileData fileData = loadFile(fullPath.c_str(), format.getChannelCount(), true, format.isFloatingPoint());
                fileDatas[mipIdx] = fileData;
                totalSize += fileData.width * fileData.height * format.getSizePerTexel();
            }

            dimensions.x = fileDatas[0].width;
            dimensions.y = fileDatas[0].height;

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = fileDatas[mipIdx].width * fileDatas[mipIdx].height * format.getSizePerTexel();
                memcpy(stagingPtr + stagingOffset, fileDatas[mipIdx].data, dataSize);
                freeData(fileDatas[mipIdx]);
                copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, stagingOffset, fileDatas[mipIdx].width, fileDatas[mipIdx].height, mipIdx);
                stagingOffset += dataSize;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
        else {
            const FileData fileData = loadFile(path, format.getChannelCount(), true, format.isFloatingPoint());

            dimensions.x = fileData.width;
            dimensions.y = fileData.height;
            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 dataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            initStagingBuffer(dataSize, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));
            memcpy(stagingPtr, fileData.data, dataSize);
            freeData(fileData);
            copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, 0, fileData.width, fileData.height, 0);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, handle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
    }

    Texture2D::Texture2D(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData) :
        Texture(format) {

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
                totalSize += datas[mipIdx].width * datas[mipIdx].height * format.getSizePerTexel();
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = datas[mipIdx].width * datas[mipIdx].height * format.getSizePerTexel();
                memcpy(stagingPtr + stagingOffset, datas[mipIdx].data, dataSize);
                copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, stagingOffset, datas[mipIdx].width, datas[mipIdx].height, mipIdx);
                stagingOffset += dataSize;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
        else {
            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 dataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            initStagingBuffer(dataSize, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));
            memcpy(stagingPtr, data, dataSize);
            copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, 0, dimensions.x, dimensions.y, 0);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, handle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
    }

    Texture2D::Texture2D(uint32 width, uint32 height, uint32 layers, TextureFormat format) :
        Texture(format) {

        dimensions.x = width;
        dimensions.y = height;
        mipLevels = 1;
        this->layers = layers;
        createImage(false, MipmapData::Options::DoNothing);
    }

    Texture2D::Texture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) :
        Texture(vkFormat) {

        BZ_ASSERT_CORE(vkImage != VK_NULL_HANDLE, "Invalid VkImage!");
        handle.imageHandle = vkImage;

        isWrapping = true;

        dimensions.x = width;
        dimensions.y = height;
        mipLevels = 1;
    }

    void Texture2D::createImage(bool hasData, MipmapData mipmapData) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(dimensions.x);
        imageInfo.extent.height = static_cast<uint32_t>(dimensions.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = layers;
        imageInfo.format = format;
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
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        BZ_ASSERT_VK(vmaCreateImage(getGraphicsContext().getMemoryAllocator(), &imageInfo, &allocInfo, &handle.imageHandle, &handle.allocationHandle, nullptr));
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<TextureCube> TextureCube::create(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) {
        auto &assetsPath = Application::get().getAssetsPath();
        return MakeRef<TextureCube>((assetsPath + basePath).c_str(), fileNames, format, mipmapData);
    }

    TextureCube::TextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) :
        Texture(format) {

        layers = 6;

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
                    const FileData fileData = loadFile(fullPath.c_str(), format.getChannelCount(), false, format.isFloatingPoint());
                    fileDatas[mipIdx * 6 + faceIdx] = fileData;
                    totalSize += fileData.width * fileData.height * format.getSizePerTexel();
                }
            }

            dimensions.x = fileDatas[0].width;
            dimensions.y = fileDatas[0].height;

            createImage(true, mipmapData);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            initStagingBuffer(totalSize, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 faceOffset = 0;
            uint32 copyOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 faceSize = fileDatas[mipIdx * 6].width * fileDatas[mipIdx * 6].height * format.getSizePerTexel();

                for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                    uint32 fileDatasIdx = mipIdx * 6 + faceIdx;
                    memcpy(stagingPtr + faceOffset, fileDatas[fileDatasIdx].data, faceSize);
                    freeData(fileDatas[fileDatasIdx]);
                    faceOffset += faceSize;
                }

                copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, copyOffset, fileDatas[mipIdx * 6].width, fileDatas[mipIdx * 6].height, mipIdx);
                copyOffset = faceOffset;
            }

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
        else {
            std::vector<FileData> fileDatas(6);
            for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                std::string fullPath = std::string(basePath) + fileNames[faceIdx];
                fileDatas[faceIdx] = loadFile(fullPath.c_str(), format.getChannelCount(), false, format.isFloatingPoint());
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
            transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            uint32 faceDataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            initStagingBuffer(faceDataSize * 6, &handle.stagingBufferHandle, &handle.stagingBufferAllocationHandle);

            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle, reinterpret_cast<void**>(&stagingPtr)));

            uint32 stagingOffset = 0;
            for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                memcpy(stagingPtr + stagingOffset, fileDatas[faceIdx].data, faceDataSize);
                freeData(fileDatas[faceIdx]);
                stagingOffset += faceDataSize;
            }
            copyBufferToImage(*this, handle.stagingBufferHandle, handle.imageHandle, commBuffer, 0, dimensions.x, dimensions.y, 0);

            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.stagingBufferAllocationHandle);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(*this, handle.imageHandle, commBuffer);
            }
            else {
                //Mipmap generation already does the transition.
                transitionImageLayout(*this, commBuffer, handle.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            submitCommandBuffer(commBuffer);
            destroyStagingBuffer(handle.stagingBufferHandle, handle.stagingBufferAllocationHandle);
        }
    }

    void TextureCube::createImage(bool hasData, MipmapData mipmapData) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(dimensions.x);
        imageInfo.extent.height = static_cast<uint32_t>(dimensions.y);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 6;
        imageInfo.format = format;
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
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        BZ_ASSERT_VK(vmaCreateImage(getGraphicsContext().getMemoryAllocator(), &imageInfo, &allocInfo, &handle.imageHandle, &handle.allocationHandle, nullptr));
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<TextureView> TextureView::create(const Ref<Texture2D> &texture2D) {
        return MakeRef<TextureView>(texture2D);
    }

    Ref<TextureView> TextureView::create(const Ref<Texture2D>& texture2D, uint32 baseLayer, uint32 layerCount) {
        return MakeRef<TextureView>(texture2D, baseLayer, layerCount);
    }

    Ref<TextureView> TextureView::create(const Ref<TextureCube> &textureCube) {
        return MakeRef<TextureView>(textureCube);
    }

    TextureView::TextureView(const Ref<Texture2D> &texture2D) :
        texture(texture2D) {
        BZ_ASSERT_CORE(texture, "Invalid Texture!");
        init(VK_IMAGE_VIEW_TYPE_2D, texture2D->getHandle().imageHandle, 0, 1);
    }

    TextureView::TextureView(const Ref<Texture2D> &texture2D, uint32 baseLayer, uint32 layerCount) :
        texture(texture2D) {
        BZ_ASSERT_CORE(texture, "Invalid Texture!");
        init(VK_IMAGE_VIEW_TYPE_2D_ARRAY, texture2D->getHandle().imageHandle, baseLayer, layerCount);
    }

    TextureView::TextureView(const Ref<TextureCube> &textureCube) :
        texture(textureCube) {
        BZ_ASSERT_CORE(texture, "Invalid Texture!");
        init(VK_IMAGE_VIEW_TYPE_CUBE, textureCube->getHandle().imageHandle, 0, 6);
    }

    TextureView::~TextureView() {
        vkDestroyImageView(getVkDevice(), handle, nullptr);
    }

    void TextureView::init(VkImageViewType viewType, VkImage vkImage, uint32 baseLayer, uint32 layerCount) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkImage;
        imageViewCreateInfo.viewType = viewType;
        imageViewCreateInfo.format = texture->getFormat();
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
        imageViewCreateInfo.subresourceRange.baseArrayLayer = baseLayer;
        imageViewCreateInfo.subresourceRange.layerCount = layerCount;
        BZ_ASSERT_VK(vkCreateImageView(getVkDevice(), &imageViewCreateInfo, nullptr, &handle));
    }


    Ref<Sampler> Sampler::Builder::build() const {
        return MakeRef<Sampler>(*this);
    }

    Sampler::Sampler(const Builder &builder) {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = builder.minFilter;
        samplerInfo.magFilter = builder.magFilter;
        samplerInfo.mipmapMode = builder.mipmapFilter;
        samplerInfo.addressModeU = builder.addressModeU;
        samplerInfo.addressModeV = builder.addressModeV;
        samplerInfo.addressModeW = builder.addressModeW;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE; //TODO
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.compareEnable = builder.compareEnabled ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = builder.compareOp;
        samplerInfo.minLod = static_cast<float>(builder.minMipmap);
        samplerInfo.maxLod = static_cast<float>(builder.maxMipmap);
        samplerInfo.borderColor = builder.borderColor;
        samplerInfo.unnormalizedCoordinates = builder.unnormalizedCoordinate ? VK_TRUE : VK_FALSE;

        BZ_ASSERT_VK(vkCreateSampler(getVkDevice(), &samplerInfo, nullptr, &handle));
    }

    Sampler::~Sampler() {
        vkDestroySampler(getVkDevice(), handle, nullptr);
    }
}