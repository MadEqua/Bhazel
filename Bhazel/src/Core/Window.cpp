#include "bzpch.h"

#include "Window.h"

#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"

#include <GLFW/glfw3.h>


namespace BZ {

void Window::init(const WindowData &data, EventCallbackFn eventCallback) {
    BZ_ASSERT_CORE(!initialized, "Window already initialized!");

    BZ_LOG_CORE_INFO("Creating Window: {0}. Dimensions: ({1}, {2})", data.title, data.dimensions.x, data.dimensions.y);

    this->data = data;
    this->eventCallback = eventCallback;

    // We will initialize Vulkan by ourselves.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(data.dimensions.x, data.dimensions.y, data.title.c_str(), nullptr, nullptr);
    BZ_CRITICAL_ERROR_CORE(window, "Could not create GLFW Window!");

    glfwSetWindowUserPointer(window, reinterpret_cast<void *>(this));

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int w, int h) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

        // Ignore minimizes and restores.
        if (w == 0 || h == 0 || (w == win.data.dimensions.x && h == win.data.dimensions.y))
            return;

        win.data.dimensions.x = w;
        win.data.dimensions.y = h;
        WindowResizedEvent event(w, h);
        win.eventCallback(event);
    });

    glfwSetWindowIconifyCallback(window, [](GLFWwindow *window, int iconified) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
        win.minimized = static_cast<bool>(iconified);
        WindowIconifiedEvent event(static_cast<bool>(iconified));
        win.eventCallback(event);
    });

    glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
        win.closed = true;
        WindowClosedEvent event;
        win.eventCallback(event);
    });

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scanCode, int action, int mods) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

        switch (action) {
            case GLFW_PRESS:
            case GLFW_REPEAT: {
                KeyPressedEvent event(key, 1);
                win.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                win.eventCallback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(window, [](GLFWwindow *window, uint32 keycode) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
        KeyTypedEvent event(keycode);
        win.eventCallback(event);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(button);
                win.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(button);
                win.eventCallback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        win.eventCallback(event);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
        Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(static_cast<int>(x), win.data.dimensions.y - static_cast<int>(y));
        win.eventCallback(event);
    });

    initialized = true;
}

void Window::destroy() {
    if (initialized) {
        glfwDestroyWindow(window);
        initialized = false;
    }
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setTitle(const char *title) {
    glfwSetWindowTitle(window, title);
}
}