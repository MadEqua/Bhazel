#include "bzpch.h"

#include "Core/Input.h"

#include "Platform/GLFW/GlfwInput.h"
#include "Platform/Win32/Win32Input.h"
#include "Graphics/Graphics.h"


namespace BZ {

    Input* Input::create(void *nativeWindowHandle) {
        switch(Graphics::api) {
        case Graphics::API::OpenGL:
        case Graphics::API::Vulkan:
            return new GlfwInput();
        case Graphics::API::D3D11:
            return new Win32Input(nativeWindowHandle);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}