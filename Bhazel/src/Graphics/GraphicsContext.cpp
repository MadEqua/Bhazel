#include "bzpch.h"

#include "GraphicsContext.h"
#include "Graphics/Graphics.h"

#include "Platform/D3D11/D3D11Context.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace BZ {

    GraphicsContext* GraphicsContext::create(void* windowHandle) {
        switch (Graphics::api) {
        /*case Graphics::API::OpenGL:
            return new OpenGLContext(windowHandle);
        case Graphics::API::D3D11:
            return new D3D11Context(windowHandle);*/
        case Graphics::API::Vulkan:
            return new VulkanContext(windowHandle);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}
