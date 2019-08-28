#include "bzpch.h"

#include "Window.h"
#include "Bhazel/Platform/GLFW/GlfwWindow.h"
#include "Bhazel/Platform/Win32/Win32Window.h"
#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    Window::Window(EventCallbackFn eventCallback, const WindowData &data) : 
        eventCallback(eventCallback), data(data) {
    }

    Window* Window::create(EventCallbackFn eventCallback, const WindowData &data) {
        switch(RendererAPI::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return new GlfwWindow(eventCallback, data);
        case RendererAPI::API::D3D11:
            return new Win32Window(eventCallback, data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}