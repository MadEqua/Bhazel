#include "bzpch.h"

#include "Input.h"
#include "Window.h"

#include "Platform/GLFW/GlfwInput.h"
#include "Platform/Win32/Win32Input.h"


namespace BZ {

    Input* Input::instance = nullptr;

    void Input::init(void *nativeWindowHandle) {
        switch(Window::getAPI())
        {
        case Window::API::GLFW:
            instance = new GlfwInput();
            break;
        case Window::API::Win32:
            instance = new Win32Input(nativeWindowHandle);
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown Window API.");
        }
    }
}