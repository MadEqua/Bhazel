#include "bzpch.h"

#include "WindowsWindow.h"

namespace BZ {

    static bool isGLFWInitialized = false;

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

            isGLFWInitialized = true;
        }

        window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), data.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, &data);
        setVSync(true);
    }

    void WindowsWindow::shutdown() {
        glfwDestroyWindow(window);
    }

    void WindowsWindow::onUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(window);
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