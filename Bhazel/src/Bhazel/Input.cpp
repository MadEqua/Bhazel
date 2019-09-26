#include "bzpch.h"

#include "Input.h"
#include "Window.h"

#include "Platform/GLFW/GlfwInput.h"
#include "Platform/Win32/Win32Input.h"
#include "Bhazel/Renderer/Renderer.h"


namespace BZ {

    Input* Input::instance = nullptr;

    void Input::init(void *nativeWindowHandle) {
        switch(Renderer::api) {
        case Renderer::API::OpenGL:
        case Renderer::API::Vulkan:
            instance = new GlfwInput();
            break;
        case Renderer::API::D3D11:
            instance = new Win32Input(nativeWindowHandle);
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
        }
    }
}