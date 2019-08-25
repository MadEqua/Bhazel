#include "bzpch.h"

#include "Window.h"
#include "Bhazel/Platform/GLFW/GlfwWindow.h"
#include "Bhazel/Platform/Win32/Win32Window.h"


namespace BZ {

    Window* Window::create(const WindowData &data) {
        switch(Window::getAPI())
        {
        case Window::API::GLFW:
            return new GlfwWindow(data);
        case Window::API::Win32:
            return new Win32Window(data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Window API.");
            return nullptr;
        }
    }
}