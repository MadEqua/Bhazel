#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"


namespace BZ {

    Shader* Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return new OpenGLShader(vertexSrc, fragmentSrc);
        case RendererAPI::API::None:
        default:
            BZ_CORE_ASSERT_ALWAYS("RendererAPI::None is currently not supported.");
            return nullptr;
        }
    }
}