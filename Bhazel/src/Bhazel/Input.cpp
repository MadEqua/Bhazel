#include "bzpch.h"

#include "Input.h"

#include "Bhazel/Platform/GLFW/GlfwInput.h"
#include "Bhazel/Platform/Win32/Win32Input.h"
#include "Bhazel/Renderer/Renderer.h"


namespace BZ {

    Input* Input::create(void *nativeWindowHandle) {
        switch(Renderer::api) {
        case Renderer::API::OpenGL:
        case Renderer::API::Vulkan:
            return new GlfwInput();
            break;
        case Renderer::API::D3D11:
            return new Win32Input(nativeWindowHandle);
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
        }
    }
}