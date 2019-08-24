#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"


namespace BZ {

    Ref<Shader> Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLShader>(vertexSrc, fragmentSrc);
        default:
            BZ_CORE_ASSERT_ALWAYS("Unknown RendererAPI.");
            return nullptr;
        }
    }
}