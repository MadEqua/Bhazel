#include "bzpch.h"

#include "Surface.h"
#include "Graphics/Internal/Instance.h"

#include <GLFW/glfw3.h>


namespace BZ {

    void Surface::init(const Instance &instance, GLFWwindow &glfwWindow) {
        this->instance = &instance;
        BZ_ASSERT_VK(glfwCreateWindowSurface(instance.getHandle(), &glfwWindow, nullptr, &handle));
    }

    void Surface::destroy() {
        vkDestroySurfaceKHR(instance->getHandle(), handle, nullptr);
    }
}