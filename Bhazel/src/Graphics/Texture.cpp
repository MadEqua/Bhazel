#include "bzpch.h"

#include "Texture.h"

#include "Graphics/Graphics.h"

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
        case TextureFormat::R8_sRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_sRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_sRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_sRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_sRGB:
            return true;
        case TextureFormat::Undefined:
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return false;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }

    bool TextureFormatWrapper::isDepthStencil() const {
        switch(format) {
        case TextureFormat::Undefined:
        case TextureFormat::R8:
        case TextureFormat::R8_sRGB:
        case TextureFormat::R8G8:
        case TextureFormat::R8G8_sRGB:
        case TextureFormat::R8G8B8:
        case TextureFormat::R8G8B8_sRGB:
        case TextureFormat::R8G8B8A8:
        case TextureFormat::R8G8B8A8_sRGB:
        case TextureFormat::B8G8R8A8:
        case TextureFormat::B8G8R8A8_sRGB:
            return false;
        case TextureFormat::D16S8:
        case TextureFormat::D24S8:
            return true;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return false;
        };
    }


    Texture::Texture(TextureFormat format) :
        format(format) {
    }

    const byte* Texture::loadFile(const char* path, bool flip, int &widthOut, int &heightOut) {
        stbi_set_flip_vertically_on_load(flip);
        int channels;
        stbi_uc* data = stbi_load(path, &widthOut, &heightOut, &channels, 4);
        BZ_CRITICAL_ERROR_CORE(data, "Failed to load image '{}'.", path);
        return static_cast<byte*>(data);
    }

    void Texture::freeData(const byte *data) {
        stbi_image_free((void*)data);
    }


    Ref<Texture2D> Texture2D::create(const std::string &path, TextureFormat format) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(assetsPath + path, format);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Texture2D> Texture2D::create(const byte *data, uint32 dataSize, uint32 width, uint32 height, TextureFormat format) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTexture2D>(data, dataSize, width, height, format);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Texture2D::Texture2D(TextureFormat format) : 
        Texture(format) {
    }


    Ref<TextureView> TextureView::create(const Ref<Texture> &texture) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureView>(texture);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<TextureView> TextureView::create(const Texture &texture) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanTextureView>(Ref<VulkanTexture2D>((VulkanTexture2D*)(&texture)));
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