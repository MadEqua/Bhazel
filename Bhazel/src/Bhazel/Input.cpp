#include "bzpch.h"

#include "Input.h"
#include "Window.h"

#include "Platform/GLFW/GlfwInput.h"
#include "Platform/Win32/Win32Input.h"

namespace BZ {

    Input* Input::instance = nullptr;

    void Input::init() {
        switch(Window::getAPI())
        {
        case Window::API::GLFW:
            instance = new GlfwInput();
            break;
        case Window::API::Win32:
            instance = new Win32Input();
            break;
        default:
            BZ_CORE_ASSERT_ALWAYS("Unknown Window API.");
        }
    }
}