#include "bzpch.h"

#include "GlfwInput.h"
#include "Bhazel/Application.h"

#include <GLFW/glfw3.h>


namespace BZ {

    bool GlfwInput::isKeyPressedImpl(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindowHandle());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool GlfwInput::isMouseButtonPressedImpl(int button) {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindowHandle());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<int, int> GlfwInput::getMousePositionImpl() {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindowHandle());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return std::make_pair(static_cast<int>(xpos), static_cast<int>(ypos));
    }

    int GlfwInput::getMouseXImpl() {
        auto[x, y] = getMousePositionImpl();
        return x;
    }

    int GlfwInput::getMouseYImpl() {
        auto[x, y] = getMousePositionImpl();
        return y;
    }
}