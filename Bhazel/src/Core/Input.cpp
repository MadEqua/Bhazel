#include "bzpch.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Input.h"

#include <GLFW/glfw3.h>


namespace BZ {

static const Window *window = nullptr;
static GLFWwindow *glfwWindow = nullptr;

void Input::init() {
    BZ::window = &Engine::get().getWindow();
    glfwWindow = BZ::window->getNativeHandle();
}

bool Input::isKeyPressed(int keycode) {
    auto state = glfwGetKey(glfwWindow, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button) {
    auto state = glfwGetMouseButton(glfwWindow, button);
    return state == GLFW_PRESS;
}

glm::ivec2 Input::getMousePosition() {
    double xpos, ypos;
    glfwGetCursorPos(glfwWindow, &xpos, &ypos);
    return glm::ivec2(static_cast<int>(xpos), window->data.dimensions.y - static_cast<int>(ypos));
}

int Input::getMouseX() {
    double xpos, ypos;
    glfwGetCursorPos(glfwWindow, &xpos, &ypos);
    return static_cast<int>(xpos);
}

int Input::getMouseY() {
    double xpos, ypos;
    glfwGetCursorPos(glfwWindow, &xpos, &ypos);
    return window->data.dimensions.y - static_cast<int>(ypos);
}
}