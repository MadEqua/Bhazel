#include "bzpch.h"

#include "InputDescription.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLInputDescription.h"
#include "Bhazel/Platform/D3D11/D3D11InputDescription.h"


namespace BZ {

    Ref<InputDescription> InputDescription::create() {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLInputDescription>();
        case Renderer::API::D3D11:
            return MakeRef<D3D11InputDescription>();
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}