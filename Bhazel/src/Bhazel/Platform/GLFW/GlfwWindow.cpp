#include "bzpch.h"

#include "GlfwWindow.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"

#include "Bhazel/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>


namespace BZ {

    static bool isGLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        BZ_LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }


    GlfwWindow::GlfwWindow(const WindowData &data) {
        init(data);
    }

    GlfwWindow::~GlfwWindow() {
        shutdown();
    }

    void GlfwWindow::init(const WindowData &data) {
        windowData = data;

        BZ_LOG_CORE_INFO("Creating GLFW Window: {0}. Dimensions: ({1}, {2})", data.title, data.width, data.height);

        if(!isGLFWInitialized) {
            glfwSetErrorCallback(GLFWErrorCallback);
            int success = glfwInit();
            BZ_ASSERT_CORE(success, "Could not intialize GLFW!");
            isGLFWInitialized = true;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#ifdef BZ_DIST
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        window = glfwCreateWindow(static_cast<int>(data.width), static_cast<int>(data.height), data.title.c_str(), nullptr, nullptr);
        BZ_ASSERT_CORE(window, "Could not create GLFW Window!");

        graphicsContext = std::make_unique<OpenGLContext>(window);

        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&windowData));

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
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow *window, uint32 keycode) {
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
            MouseMovedEvent event(static_cast<int>(x), static_cast<int>(y));
            data.eventCallback(event);
        });
    }

    void GlfwWindow::shutdown() {
        if(isGLFWInitialized) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    }

    void GlfwWindow::onUpdate() {
        glfwPollEvents();
        graphicsContext->swapBuffers();
    }
}