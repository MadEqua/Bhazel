#include "bzpch.h"

#include "Framebuffer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Renderer/Renderer.h"

#include "Bhazel/Platform/Vulkan/VulkanFramebuffer.h"


namespace BZ {

    Ref<Framebuffer> Framebuffer::create(const std::vector<Ref<TextureView>> &textureViews) {
        switch(Renderer::api) {
        /*case Renderer::API::OpenGL:
            return MakeRef<OpenGLTexture2D>(assetsPath + path);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanFramebuffer>(textureViews);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Framebuffer::Framebuffer(const std::vector<Ref<TextureView>> &textureViews) :
        textureViews(textureViews) {

        BZ_ASSERT_CORE(!textureViews.empty(), "Empty TextureView list!");
    }
}