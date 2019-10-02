#include "bzpch.h"

#include "Texture.h"

#include "Bhazel/Application.h"
#include "Bhazel/Renderer/Renderer.h"

//#include "Bhazel/Platform/D3D11/D3D11Texture.h"
//#include "Bhazel/Platform/OpenGL/OpenGLTexture.h"
#include "Bhazel/Platform/Vulkan/VulkanTexture.h"

#include <stb_image.h>


namespace BZ {

    Texture::Texture(TextureFormat format) :
        format(format) {
    }

    const byte* Texture::loadFile(const char* path, bool flip, int &widthOut, int &heightOut) {
        int channels;
        stbi_uc* data = stbi_load(path, &widthOut, &heightOut, &channels, 4);
        BZ_ASSERT_CORE(data, "Failed to load image '{}'.", path);
        return static_cast<byte*>(data);
    }


    Ref<Texture2D> Texture2D::create(const std::string &path, TextureFormat format) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Renderer::api) {
        /*case Renderer::API::OpenGL:
            return MakeRef<OpenGLTexture2D>(assetsPath + path);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanTexture2D>(assetsPath + path, format);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Texture2D::Texture2D(TextureFormat format) : 
        Texture(format) {
    }


    Ref<TextureView> TextureView::create(const Ref<Texture>& texture) {
        switch(Renderer::api) {
            /*case Renderer::API::OpenGL:
                return MakeRef<OpenGLTexture2D>(assetsPath + path);
            case Renderer::API::D3D11:
                return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanTextureView>(texture);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    TextureView::TextureView(const Ref<Texture> &texture) :
        texture(texture) {
    }
}