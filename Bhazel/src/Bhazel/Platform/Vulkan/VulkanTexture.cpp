#include "bzpch.h"

#include "VulkanTexture.h"
#include "Bhazel/Platform/Vulkan/VulkanConversions.h"


namespace BZ {

    Ref<VulkanTexture2D> VulkanTexture2D::wrap(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) {
        return MakeRef<VulkanTexture2D>(vkImage, width, height, vkFormat);
    }

    VulkanTexture2D::VulkanTexture2D(const std::string& path, TextureFormat format) :
        Texture2D(format), isWrapping(false) {

        int width, height;
        const byte *data = loadFile(path.c_str(), true, width, height);

        dimensions.x = width;
        dimensions.y = height;

        //TODO
    }

    VulkanTexture2D::VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height, VkFormat vkFormat) :
        Texture2D(vkFormatToTextureFormat(vkFormat)), isWrapping(true) {

        BZ_ASSERT_CORE(vkImage != VK_NULL_HANDLE, "Invalid VkImage!");
        nativeHandle = vkImage;

        dimensions.x = width;
        dimensions.y = height;
    }

    VulkanTexture2D::~VulkanTexture2D() {
        if(!isWrapping)
            vkDestroyImage(getDevice(), nativeHandle, nullptr);
    }


    VulkanTextureView::VulkanTextureView(const Ref<Texture> &texture) :
        TextureView(texture) {

        //TODO: fill correctly
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = static_cast<VulkanTexture2D &>(*texture).getNativeHandle();
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = textureFormatToVk(texture->getFormat());
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        BZ_ASSERT_VK(vkCreateImageView(getDevice(), &imageViewCreateInfo, nullptr, &nativeHandle));
    }

    VulkanTextureView::~VulkanTextureView() {
        vkDestroyImageView(getDevice(), nativeHandle, nullptr);
    }
}