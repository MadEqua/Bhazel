#include "bzpch.h"

#include "Texture.h"
#include "Renderer.h"

#include "Bhazel/Application.h"

#include "Bhazel/Platform/OpenGL/OpenGLTexture.h"
#include "Bhazel/Platform/D3D11/D3D11Texture.h"


namespace BZ {

    Ref<Texture2D> Texture2D::create(const std::string &path) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Renderer::api) {
        /*case Renderer::API::OpenGL:
            return MakeRef<OpenGLTexture2D>(assetsPath + path);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}