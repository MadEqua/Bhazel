#include "bzpch.h"

#include "InputDescription.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLInputDescription.h"

namespace BZ {

    Ref<InputDescription> InputDescription::create() {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLInputDescription>();
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}