#include "bzpch.h"

#include "GraphicsContext.h"
#include "Bhazel/Renderer/Renderer.h"

#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "Bhazel/Platform/OpenGL/OpenGLContext.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"


namespace BZ {

    GraphicsContext* GraphicsContext::create(void* windowHandle) {
        switch (Renderer::api) {
        /*case Renderer::API::OpenGL:
            return new OpenGLContext(windowHandle);
        case Renderer::API::D3D11:
            return new D3D11Context(windowHandle);*/
        case Renderer::API::Vulkan:
            return new VulkanContext(windowHandle);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}
