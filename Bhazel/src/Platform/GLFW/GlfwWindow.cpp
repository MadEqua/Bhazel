#include "bzpch.h"

#include "Core/Window.h"

#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

#include <GLFW/glfw3.h>


namespace BZ {

    static bool isGLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        BZ_LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window::Window(const WindowData &data, EventCallbackFn eventCallback) :
        Window(data, eventCallback) {
        BZ_LOG_CORE_INFO("Creating GLFW Window: {0}. Dimensions: ({1}, {2})", data.title, data.dimensions.x, data.dimensions.y);

        if(!isGLFWInitialized) {
            glfwSetErrorCallback(GLFWErrorCallback);
            int success = glfwInit();
            BZ_ASSERT_CORE(success, "Could not intialize GLFW!");
            isGLFWInitialized = true;
        }

#ifdef BZ_PLATFORM_OPENGL43
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef BZ_DIST
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#elif defined BZ_PLATFORM_VULKAN
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

        GLFWwindow *glfwWindow = glfwCreateWindow(data.dimensions.x, data.dimensions.y, data.title.c_str(), nullptr, nullptr);
        BZ_ASSERT_CORE(glfwWindow, "Could not create GLFW Window!");

        glfwSetWindowUserPointer(glfwWindow, reinterpret_cast<void *>(this));

        glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow *window, int w, int h) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

            //Ignore minimizes and restores.
            if(w == 0 || h == 0 || (w == win.data.dimensions.x && h == win.data.dimensions.y)) return;

            win.data.dimensions.x = w;
            win.data.dimensions.y = h;
            WindowResizedEvent event(w, h);
            win.eventCallback(event);
        });

        glfwSetWindowIconifyCallback(glfwWindow, [](GLFWwindow *window, int iconified) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
            win.minimized = static_cast<bool>(iconified);
            WindowIconifiedEvent event(static_cast<bool>(iconified));
            win.eventCallback(event);
        });

        glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow *window) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
            win.closed = true;
            WindowClosedEvent event;
            win.eventCallback(event);
        });

        glfwSetKeyCallback(glfwWindow, [](GLFWwindow *window, int key, int scanCode, int action, int mods) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

            switch(action) {
                case GLFW_PRESS:
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    win.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    win.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(glfwWindow, [](GLFWwindow *window, uint32 keycode) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(keycode);
            win.eventCallback(event);
        });

        glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow *window, int button, int action, int mods) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));

            switch(action) {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event(button);
                win.eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event(button);
                win.eventCallback(event);
                break;
            }
            }
        });

        glfwSetScrollCallback(glfwWindow, [](GLFWwindow *window, double xOffset, double yOffset) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            win.eventCallback(event);
        });

        glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow *window, double x, double y) {
            Window &win = *static_cast<Window *>(glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<int>(x), static_cast<int>(y));
            win.eventCallback(event);
        });

        nativeHandle = glfwWindow;
    }

    Window::~Window() {
        if(isGLFWInitialized) {
            glfwDestroyWindow(static_cast<GLFWwindow*>(nativeHandle));
            glfwTerminate();
        }
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::setTitle(const char* title) {
        glfwSetWindowTitle(static_cast<GLFWwindow*>(nativeHandle), title);
    }
}