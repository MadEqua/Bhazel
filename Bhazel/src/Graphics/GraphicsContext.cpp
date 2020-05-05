#include "bzpch.h"

#include "GraphicsContext.h"
#include "Graphics/Graphics.h"

#include "Platform/Vulkan/VulkanContext.h"


namespace BZ {

    GraphicsContext* GraphicsContext::create(void* windowHandle) {
        switch (Graphics::api) {
        case Graphics::API::Vulkan:
            return new VulkanContext(windowHandle);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}
