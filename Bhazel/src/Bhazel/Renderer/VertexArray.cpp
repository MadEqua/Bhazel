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
        default:
            BZ_CORE_ASSERT_ALWAYS("Unknown RendererAPI.");
            return nullptr;
        }
    }
}