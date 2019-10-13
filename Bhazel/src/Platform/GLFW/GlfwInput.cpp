#include "bzpch.h"

#include "Core/Input.h"
#include <GLFW/glfw3.h>


namespace BZ {

    bool Input::isKeyPressed(int keycode) {
        auto window = static_cast<GLFWwindow*>(nativeWindowHandle);
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::isMouseButtonPressed(int button) {
        auto window = static_cast<GLFWwindow *>(nativeWindowHandle);
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    glm::ivec2 Input::getMousePosition() {
        auto window = static_cast<GLFWwindow *>(nativeWindowHandle);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos));
    }

    int Input::getMouseX() {
        return getMousePosition().x;
    }

    int Input::getMouseY() {
        return getMousePosition().y;
    }
}