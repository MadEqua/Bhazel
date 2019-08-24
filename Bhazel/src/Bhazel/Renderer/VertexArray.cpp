#include "bzpch.h"

#include "VertexArray.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLVertexArray.h"

namespace BZ {

    Ref<VertexArray> VertexArray::create() {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLVertexArray>();
        case RendererAPI::API::None:
        default:
            BZ_CORE_ASSERT_ALWAYS("RendererAPI::None is currently not supported.");
            return nullptr;
        }
    }
}