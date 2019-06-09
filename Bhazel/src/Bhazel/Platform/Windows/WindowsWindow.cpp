#include "bzpch.h"

#include "WindowsWindow.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"

#include "Bhazel/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

namespace BZ {

    static bool isGLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        BZ_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window* Window::create(const WindowProps &props) {
        return new WindowsWindow(props);
    }

    WindowsWindow::WindowsWindow(const WindowProps &props) {
        init(props);
    }

    WindowsWindow::~WindowsWindow() {
        shutdown();
    }

    void WindowsWindow::init(const WindowProps &props) {
        data.title = props.title;
        data.width = props.width;
        data.height = props.height;

        BZ_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

        if(!isGLFWInitialized) {
            // TODO: glfwTerminate on system shutdown
            int success = glfwInit();
            BZ_CORE_ASSERT(success, "Could not intialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);
            isGLFWInitialized = true;
        }

        window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), data.title.c_str(), nullptr, nullptr);
        
        graphicsContext = new OpenGLContext(window);
        graphicsContext->init();

        glfwSetWindowUserPointer(window, &data);
        setVSync(true);

        glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int w, int h) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.width = w;
            data.height = h;

            WindowResizeEvent event(w, h);
            data.eventCallback(event);
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent event;
            data.eventCallback(event);
        });

        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scanCode, int action, int mods) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch(action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow *window, unsigned int keycode) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(keycode);
            data.eventCallback(event);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch(action)
            {
                case GLFW_PRESS: 
                {
                    MouseButtonPressedEvent event(button);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.eventCallback(event);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
            WindowData &data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<float>(x), static_cast<float>(y));
            data.eventCallback(event);
        });
    }

    void WindowsWindow::shutdown() {
        glfwDestroyWindow(window);
        delete graphicsContext;
    }

    void WindowsWindow::onUpdate() {
        glfwPollEvents();
        graphicsContext->swapBuffers();
    }

    void WindowsWindow::setVSync(bool enabled) {
        if(enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        data.vSync = enabled;
    }

    bool WindowsWindow::isVSync() const {
        return data.vSync;
    }
}