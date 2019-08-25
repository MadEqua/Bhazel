#include "bzpch.h"

#include "Input.h"
#include "Window.h"

#include "Platform/GLFW/GlfwInput.h"
#include "Platform/Win32/Win32Input.h"
#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    Input* Input::instance = nullptr;

    void Input::init(void *nativeWindowHandle) {
        switch(RendererAPI::getAPI())
        {
        case RendererAPI::API::OpenGL:
            instance = new GlfwInput();
            break;
        case RendererAPI::API::D3D11:
            instance = new Win32Input(nativeWindowHandle);
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Window API.");
        }
    }
}