#include "bzpch.h"

#include "Texture.h"

#include "Graphics/Graphics.h"
#include "Core/Application.h"

#include "Platform/Vulkan/VulkanTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace BZ {

    TextureFormat::TextureFormat(TextureFormatEnum formatEnum) :
        formatEnum(formatEnum)  {
    }

    bool TextureFormat::isColor() const {
        switch(formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return true;
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
            return false;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isDepth() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return false;
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isStencil() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return false;
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isDepthStencil() const {
        switch(formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return false;
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isDepthOnly() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return false;
        case TextureFormatEnum::D32_SFLOAT:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isSRGB() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return false;
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8_SRGB:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    bool TextureFormat::isFloatingPoint() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8_SRGB:
            return false;
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return true;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    int TextureFormat::getChannelCount() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
            return 1;
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
            return 2;
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
            return 3;
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::B8G8R8A8_SRGB:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return 4;
        case TextureFormatEnum::D16S8:
        case TextureFormatEnum::D24S8:
            return 2;
        case TextureFormatEnum::D32_SFLOAT:
            return 1;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return false;
        };
    }

    int TextureFormat::getSizePerChannel() const {
        switch (formatEnum) {
        case TextureFormatEnum::R8:
        case TextureFormatEnum::R8G8:
        case TextureFormatEnum::R8G8B8:
        case TextureFormatEnum::R8G8B8A8:
        case TextureFormatEnum::B8G8R8A8:
        case TextureFormatEnum::R8_SRGB:
        case TextureFormatEnum::R8G8_SRGB:
        case TextureFormatEnum::R8G8B8_SRGB:
        case TextureFormatEnum::R8G8B8A8_SRGB:
        case TextureFormatEnum::B8G8R8A8_SRGB:
            return 1;
        case TextureFormatEnum::R16_SFLOAT:
        case TextureFormatEnum::R16G16_SFLOAT:
        case TextureFormatEnum::R16G16B16_SFLOAT:
        case TextureFormatEnum::R16G16B16A16_SFLOAT:
            return 2;
        case TextureFormatEnum::D32_SFLOAT:
        case TextureFormatEnum::R32_SFLOAT:
        case TextureFormatEnum::R32G32_SFLOAT:
        case TextureFormatEnum::R32G32B32_SFLOAT:
        case TextureFormatEnum::R32G32B32A32_SFLOAT:
            return 4;
        case TextureFormatEnum::D24S8:
        case TextureFormatEnum::D16S8:
            BZ_ASSERT_ALWAYS_CORE("No uniform size per channel!");
            return 0;
        case TextureFormatEnum::Undefined:
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return 0;
        };
    }

    int TextureFormat::getSizePerTexel() const {
        return getChannelCount() * getSizePerChannel();
    }


    Texture::Texture(TextureFormat format) :
        format(format) {
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


    Ref<Texture2D> Texture2D::create(const char *path, TextureFormat format, MipmapData mipmapData) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>((assetsPath + path).c_str(), format, mipmapData);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::create(const byte *data, uint32 width, uint32 height, TextureFormat format, MipmapData mipmapData) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(data, width, height, format, mipmapData);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::createRenderTarget(uint32 width, uint32 height, uint32 layers, TextureFormat format) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(width, height, layers, format);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Texture2D::Texture2D(TextureFormat format) :
        Texture(format) {
    }


    Ref<TextureCube> TextureCube::create(const char *basePath, const char *fileNames[6], TextureFormat format, MipmapData mipmapData) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureCube>((assetsPath + basePath).c_str(), fileNames, format, mipmapData);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    TextureCube::TextureCube(TextureFormat format) :
        Texture(format) {
    }


    Ref<TextureView> TextureView::create(const Ref<Texture2D> &texture2D) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureView>(texture2D);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<TextureView> TextureView::create(const Ref<Texture2D>& texture2D, uint32 baseLayer, uint32 layerCount) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            BZ_ASSERT_CORE(texture2D->getLayers() <= baseLayer + layerCount, "Texture2D doesn't have enough layers!");
            return MakeRef<VulkanTextureView>(texture2D, baseLayer, layerCount);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<TextureView> TextureView::create(const Ref<TextureCube> &textureCube) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureView>(textureCube);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    TextureView::TextureView(const Ref<Texture> &texture) :
        texture(texture) {
        BZ_ASSERT_CORE(texture, "Invalid Texture!");
    }


    Ref<Sampler> Sampler::Builder::build() const {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanSampler>(*this);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}