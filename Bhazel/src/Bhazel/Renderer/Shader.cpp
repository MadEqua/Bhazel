#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"


namespace BZ {

    Ref<Shader> Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(vertexSrc, fragmentSrc);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(vertexSrc, fragmentSrc);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}