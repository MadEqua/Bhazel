#include "bzpch.h"

#include "Texture.h"

#include "Core/Application.h"
#include "Core/Utils.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/CommandBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace BZ {

    static void generateMipmaps(CommandBuffer &comBuffer, const Texture &texture) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(BZ_GRAPHICS_DEVICE.getPhysicalDevice().getHandle(), texture.getFormat(), &formatProperties);

        BZ_CRITICAL_ERROR(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
            "Linear interpolation for blitting is not supported. Cannot generate mipmaps!");

        int mipWidth = texture.getDimensions().x;
        int mipHeight = texture.getDimensions().y;

        uint32 i;
        for (i = 1; i < texture.getMipLevels(); ++i) {

            //Layout transition previous mipmap from DST_OPTIMAL to SRC_OPTIMAL.
            comBuffer.pipelineBarrierTexture(texture,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                i - 1, 1);

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

            comBuffer.blitTexture(texture, texture, 
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &blit, 1, VK_FILTER_LINEAR);

            //Layout transition previous mipmap from SRC_OPTIMAL to SHADER_READ_ONLY_OPTIMAL.
            comBuffer.pipelineBarrierTexture(texture,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                i - 1, 1);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        //Layout transition last mipmap from DST_OPTIMAL to SHADER_READ_ONLY_OPTIMAL.
        comBuffer.pipelineBarrierTexture(texture,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            i - 1, 1);
    }

    static void copyBufferToImage(CommandBuffer &comBuffer, const Buffer &buffer, const Texture &texture, uint32 bufferOffset, uint32 mipLevel, uint32 width, uint32 height) {
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

        comBuffer.copyBufferToTexture(buffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &region, 1);
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
            vmaDestroyImage(BZ_MEM_ALLOCATOR, handle.imageHandle, handle.allocationHandle);
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

        //Graphics queue (and not transfer) because of the layout transition operations.
        CommandBuffer &commBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        BufferPtr stagingPtr;

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

            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                              0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);

            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, totalSize, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = fileDatas[mipIdx].width * fileDatas[mipIdx].height * format.getSizePerTexel();
                memcpy(stagingPtr + stagingOffset, fileDatas[mipIdx].data, dataSize);
                freeData(fileDatas[mipIdx]);
                copyBufferToImage(commBuffer, stagingBuffer, *this, stagingOffset, mipIdx, fileDatas[0].width, fileDatas[0].height);
                stagingOffset += dataSize;
            }

            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                               VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
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
            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                               0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);

            uint32 dataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, dataSize, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);
            memcpy(stagingPtr, fileData.data, dataSize);
            freeData(fileData);

            copyBufferToImage(commBuffer, stagingBuffer, *this, 0, 0, fileData.width, fileData.height);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(commBuffer, *this);
            }
            else {
                //Mipmap generation already does the transition.
                commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                   VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            }
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
        }
    }

    Texture2D::Texture2D(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData) :
        Texture(format) {

        //Graphics queue (and not transfer) because of the layout transition operations.
        CommandBuffer &commBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        BufferPtr stagingPtr;

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
            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                               0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);

            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, totalSize, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);

            uint32 stagingOffset = 0;
            for (uint32 mipIdx = 0; mipIdx < mipLevels; ++mipIdx) {
                uint32 dataSize = datas[mipIdx].width * datas[mipIdx].height * format.getSizePerTexel();
                memcpy(stagingPtr + stagingOffset, datas[mipIdx].data, dataSize);
                copyBufferToImage(commBuffer, stagingBuffer, *this, stagingOffset, mipIdx, width, height);
                stagingOffset += dataSize;
            }

            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
        }
        else {
            if (mipmapData.option == MipmapData::Options::Generate) {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimensions.x, dimensions.y)))) + 1;
            }
            else {
                mipLevels = 1;
            }

            createImage(true, mipmapData);
            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                              0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);
            
            uint32 dataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, dataSize, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);
            memcpy(stagingPtr, data, dataSize);

            copyBufferToImage(commBuffer, stagingBuffer, *this, 0, 0, width, height);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(commBuffer, *this);
            }
            else {
                //Mipmap generation already does the transition.
                commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            }
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
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
        BZ_ASSERT_VK(vmaCreateImage(BZ_MEM_ALLOCATOR, &imageInfo, &allocInfo, &handle.imageHandle, &handle.allocationHandle, nullptr));
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<TextureCube> TextureCube::create(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) {
        auto &assetsPath = Application::get().getAssetsPath();
        return MakeRef<TextureCube>((assetsPath + basePath).c_str(), fileNames, format, mipmapData);
    }

    TextureCube::TextureCube(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) :
        Texture(format) {

        layers = 6;

        //Graphics queue (and not transfer) because of the layout transition operations.
        CommandBuffer &commBuffer = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        BufferPtr stagingPtr;

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
            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                              0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);

            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, totalSize, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);

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

                copyBufferToImage(commBuffer, stagingBuffer, *this, copyOffset, mipIdx, fileDatas[mipIdx * 6].width, fileDatas[mipIdx * 6].height);
                copyOffset = faceOffset;
            }

            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
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
            commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                              0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels);

            uint32 faceDataSize = dimensions.x * dimensions.y * format.getSizePerTexel();
            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, faceDataSize * 6, MemoryType::Staging, nullptr);
            stagingPtr = stagingBuffer.map(0);

            uint32 stagingOffset = 0;
            for (uint32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
                memcpy(stagingPtr + stagingOffset, fileDatas[faceIdx].data, faceDataSize);
                freeData(fileDatas[faceIdx]);
                stagingOffset += faceDataSize;
            }
            copyBufferToImage(commBuffer, stagingBuffer, *this, 0, 0, fileDatas[0].width, fileDatas[0].height);

            if (mipmapData.option == MipmapData::Options::Generate) {
                generateMipmaps(commBuffer, *this);
            }
            else {
                //Mipmap generation already does the transition.
                commBuffer.pipelineBarrierTexture(*this, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mipLevels);
            }
            commBuffer.endAndSubmitImmediately();
            BZ_GRAPHICS_CTX.waitForQueue(QueueProperty::Graphics);
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
        BZ_ASSERT_VK(vmaCreateImage(BZ_MEM_ALLOCATOR, &imageInfo, &allocInfo, &handle.imageHandle, &handle.allocationHandle, nullptr));
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
        vkDestroyImageView(BZ_GRAPHICS_DEVICE.getHandle(), handle, nullptr);
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
        BZ_ASSERT_VK(vkCreateImageView(BZ_GRAPHICS_DEVICE.getHandle(), &imageViewCreateInfo, nullptr, &handle));
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

        BZ_ASSERT_VK(vkCreateSampler(BZ_GRAPHICS_DEVICE.getHandle(), &samplerInfo, nullptr, &handle));
    }

    Sampler::~Sampler() {
        vkDestroySampler(BZ_GRAPHICS_DEVICE.getHandle(), handle, nullptr);
    }
}