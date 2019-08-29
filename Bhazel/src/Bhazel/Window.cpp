#include "bzpch.h"

#include "Window.h"
#include "Bhazel/Platform/GLFW/GlfwWindow.h"
#include "Bhazel/Platform/Win32/Win32Window.h"
#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    Window::Window(const WindowData &data, EventCallbackFn eventCallback) :
        eventCallback(eventCallback), data(data) {
    }

    Window* Window::create(const WindowData &data, EventCallbackFn eventCallback) {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return new GlfwWindow(data, eventCallback);
        case Renderer::API::D3D11:
            return new Win32Window(data, eventCallback);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}