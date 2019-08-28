#include "bzpch.h"

#include "Texture.h"
#include "Renderer.h"
#include "Bhazel/Platform/OpenGL/OpenGLTexture.h"
#include "Bhazel/Platform/D3D11/D3D11Texture.h"


namespace BZ {

    Ref<Texture2D> Texture2D::create(const std::string &path) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLTexture2D>(path);
        case RendererAPI::API::D3D11:
            return MakeRef<D3D11Texture2D>(path);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}