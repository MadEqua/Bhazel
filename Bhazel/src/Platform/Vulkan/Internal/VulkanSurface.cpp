#include "bzpch.h"

#include  "VulkanSurface.h"

#include <GLFW/glfw3.h>


namespace BZ {

    /*VulkanSurface::VulkanSurface(VkInstance instance, GLFWwindow &glfwWindow) {
        init(instance, glfwWindow);
    }

    VulkanSurface::~VulkanSurface() {
        destroy();
    }*/

    void VulkanSurface::init(VkInstance instance, GLFWwindow &glfwWindow) {
        BZ_ASSERT_CORE(surface == VK_NULL_HANDLE, "Surface is already inited!");

        this->instance = instance;
        BZ_ASSERT_VK(glfwCreateWindowSurface(instance, &glfwWindow, nullptr, &surface));
    }

    void VulkanSurface::destroy() {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
}