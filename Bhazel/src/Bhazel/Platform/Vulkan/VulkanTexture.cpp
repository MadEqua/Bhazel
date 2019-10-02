#include "bzpch.h"

#include "VulkanTexture.h"

#include "Bhazel/Application.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"


namespace BZ {

    Ref<VulkanTexture2D> VulkanTexture2D::create(VkImage vkImage, uint32 width, uint32 height) {
        return MakeRef<VulkanTexture2D>(vkImage, width, height);
    }

    VulkanTexture2D::VulkanTexture2D(const std::string& path, TextureFormat format) :
        context(static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext())),
        Texture2D(format) {

        int width, height;
        const byte *data = loadFile(path.c_str(), true, width, height);

        dimensions.x = width;
        dimensions.y = height;

        //TODO
    }

    VulkanTexture2D::VulkanTexture2D(VkImage vkImage, uint32 width, uint32 height) :
        context(static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext())),
        imageHandle(vkImage),
        Texture2D(TextureFormat::Unknown) {

        BZ_ASSERT_CORE(vkImage != VK_NULL_HANDLE, "Invalid vkImage!");

        //TODO
        dimensions.x = width;
        dimensions.y = height;
    }

    VulkanTexture2D::~VulkanTexture2D() {
        vkDestroyImage(context.getDevice(), imageHandle, nullptr);
    }

    VulkanTextureView::VulkanTextureView(const Ref<Texture> &texture) :
        context(static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext())),
        TextureView(texture) {

        BZ_ASSERT_CORE(texture, "Invalid Texture reference!");

        //TODO: this cast is bad
        VulkanTexture2D &vkTexture = static_cast<VulkanTexture2D&>(*texture);

        //TODO: fill correctly
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkTexture.imageHandle;
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
        BZ_ASSERT_VK(vkCreateImageView(context.getDevice(), &imageViewCreateInfo, nullptr, &imageViewHandle));
    }

    VkFormat textureFormatToVk(TextureFormat format) {
        switch(format) {
        case TextureFormat::Unknown:
            //TODO
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return VK_FORMAT_UNDEFINED;
        }
    }
}