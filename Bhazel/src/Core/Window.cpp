#include "bzpch.h"

#include "Window.h"
#include "Platform/GLFW/GlfwWindow.h"
#include "Platform/Win32/Win32Window.h"
#include "Graphics/Graphics.h"


namespace BZ {

    Window::Window(const WindowData &data, EventCallbackFn eventCallback) :
        eventCallback(eventCallback), data(data) {
    }

    Window* Window::create(const WindowData &data, EventCallbackFn eventCallback) {
        switch(Graphics::api) {
        case Graphics::API::OpenGL:
        case Graphics::API::Vulkan:
            return new GlfwWindow(data, eventCallback);
        case Graphics::API::D3D11:
            return new Win32Window(data, eventCallback);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Renderer API.");
            return nullptr;
        }
    }
}