#include "bzpch.h"

#include "GraphicsContext.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "Bhazel/Platform/OpenGL/OpenGLContext.h"


namespace BZ {

    GraphicsContext* GraphicsContext::create(void* windowHandle) {
        switch (Renderer::api) {
        case Renderer::API::OpenGL:
            return new OpenGLContext(windowHandle);
        case Renderer::API::D3D11:
            return new D3D11Context(windowHandle);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}
