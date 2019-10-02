#include "bzpch.h"

#include "GlfwInput.h"
#include "Bhazel/Application.h"

#include <GLFW/glfw3.h>


namespace BZ {

    bool GlfwInput::isKeyPressed(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeHandle());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool GlfwInput::isMouseButtonPressed(int button) {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeHandle());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    glm::ivec2 GlfwInput::getMousePosition() {
        auto window = static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeHandle());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos));
    }

    int GlfwInput::getMouseX() {
        return getMousePosition().x;
    }

    int GlfwInput::getMouseY() {
        return getMousePosition().y;
    }
}