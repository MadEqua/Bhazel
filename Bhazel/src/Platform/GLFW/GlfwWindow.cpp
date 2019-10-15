#include "bzpch.h"

#include "GlfwWindow.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Graphics/Graphics.h"

#include <GLFW/glfw3.h>


namespace BZ {

    static bool isGLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        BZ_LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    GlfwWindow::GlfwWindow(const WindowData &data, EventCallbackFn eventCallback) :
        Window(data, eventCallback) {
        init();
    }

    GlfwWindow::~GlfwWindow() {
        shutdown();
    }

    void GlfwWindow::init() {
        BZ_LOG_CORE_INFO("Creating GLFW Window: {0}. Dimensions: ({1}, {2})", data.title, data.dimensions.x, data.dimensions.y);

        if(!isGLFWInitialized) {
            glfwSetErrorCallback(GLFWErrorCallback);
            int success = glfwInit();
            BZ_ASSERT_CORE(success, "Could not intialize GLFW!");
            isGLFWInitialized = true;
        }

        if(Graphics::api == Graphics::API::OpenGL) {
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
        }
        else { //Vulkan
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window = glfwCreateWindow(data.dimensions.x, data.dimensions.y, data.title.c_str(), nullptr, nullptr);
        BZ_ASSERT_CORE(window, "Could not create GLFW Window!");  

        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
            GlfwWindow& win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            
            //Ignore minimizes and restores.
            if (w == 0 || h == 0 || (w == win.data.dimensions.x && h == win.data.dimensions.y)) return;

            win.data.dimensions.x = w;
            win.data.dimensions.y = h;
            WindowResizedEvent event(w, h);
            win.eventCallback(event);
         });

        glfwSetWindowIconifyCallback(window, [](GLFWwindow* window, int iconified) {
            GlfwWindow& win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            win.minimized = static_cast<bool>(iconified);
            WindowIconifiedEvent event(static_cast<bool>(iconified));
            win.eventCallback(event);
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow *window) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            win.closed = true;
            WindowClosedEvent event;
            win.eventCallback(event);
        });

        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scanCode, int action, int mods) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

            switch(action)
            {
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

        glfwSetCharCallback(window, [](GLFWwindow *window, uint32 keycode) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(keycode);
            win.eventCallback(event);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

            switch(action)
            {
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

        glfwSetScrollCallback(window, [](GLFWwindow *window, double xOffset, double yOffset) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            win.eventCallback(event);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
            GlfwWindow &win = *static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<int>(x), static_cast<int>(y));
            win.eventCallback(event);
        });
    }

    void GlfwWindow::shutdown() {
        if(isGLFWInitialized) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    }

    void GlfwWindow::pollEvents() {
        glfwPollEvents();
    }

    void GlfwWindow::setTitle(const char* title) {
        glfwSetWindowTitle(window, title);
    }
}