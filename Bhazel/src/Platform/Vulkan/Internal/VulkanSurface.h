#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"

struct GLFWwindow;


namespace BZ {

    class VulkanSurface {
    public:
        VulkanSurface() = default;
        //VulkanSurface(VkInstance instance, GLFWwindow &glfwWindow);
        //~VulkanSurface();

        void init(VkInstance instance, GLFWwindow &glfwWindow);
        void destroy();

        VkSurfaceKHR getNativeHandle() const { return surface; }

    private:
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkInstance instance = VK_NULL_HANDLE;
    };
}