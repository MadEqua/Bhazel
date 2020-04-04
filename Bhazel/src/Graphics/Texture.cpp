#include "bzpch.h"

#include "Texture.h"

#include "Graphics/Graphics.h"
#include "Core/Application.h"

//#include "Platform/D3D11/D3D11Texture.h"
//#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include <stb_image.h>


namespace BZ {

    TextureFormatWrapper::TextureFormatWrapper(TextureFormat format) :
        format(format)  {
    }

    bool TextureFormatWrapper::isColor() const {
        switch(format) {
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
            return true;
        case TextureFormat::Undefined:
        case TextureFormat::D32:
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return false;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isDepth() const {
        switch (format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
            return false;
        case TextureFormat::D32:
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isStencil() const {
        switch (format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
        case TextureFormat::D32:
            return false;
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isDepthStencil() const {
        switch(format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
        case TextureFormat::D32:
            return false;
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isDepthOnly() const {
        switch (format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return false;
        case TextureFormat::D32:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isSRGB() const {
        switch (format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
        case TextureFormat::D32:
            return false;
        case TextureFormat::R8_SRGB:
        case TextureFormat::R8G8_SRGB:
        case TextureFormat::R8G8B8_SRGB:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8_SRGB:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    int TextureFormatWrapper::getChannelCount() const {
        switch (format) {
        case TextureFormat::Undefined:
            return 0;
        case TextureFormat::R8:
        case TextureFormat::R8_SRGB:
            return 1;
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_SRGB:
            return 2;
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_SRGB:
            return 3;
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_SRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_SRGB:
            return 4;
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return 2;
        case TextureFormat::D32:
            return 1;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }


    Texture::Texture(TextureFormat format) :
        format(format) {
    }

    const byte* Texture::loadFile(const char* path, int desiredChannels, bool flip, int &widthOut, int &heightOut) {
        stbi_set_flip_vertically_on_load(flip);
        int channelsInFile;
        stbi_uc* data = stbi_load(path, &widthOut, &heightOut, &channelsInFile, desiredChannels);
        BZ_CRITICAL_ERROR_CORE(data, "Failed to load image '{}'.", path);
        return static_cast<byte*>(data);
    }

    void Texture::freeData(const byte *data) {
        stbi_image_free((void*)data);
    }


    Ref<Texture2D> Texture2D::create(const char* path, TextureFormat format, bool generateMipmaps) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>((assetsPath + path).c_str(), format, generateMipmaps);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::create(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format, bool generateMipmaps) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(data, dataSize, width, height, format, generateMipmaps);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::createRenderTarget(uint32 width, uint32 height, TextureFormat format) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(width, height, format);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Texture2D::Texture2D(TextureFormat format) :
        Texture(format) {
    }


    Ref<TextureCube> TextureCube::create(const char *basePath, const char *fileNames[6], TextureFormat format, bool generateMipmaps) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureCube>((assetsPath + basePath).c_str(), fileNames, format, generateMipmaps);
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