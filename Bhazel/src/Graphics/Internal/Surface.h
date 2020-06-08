#pragma once

#include "Graphics/Internal/VulkanIncludes.h"

struct GLFWwindow;


namespace BZ {

class Instance;

class Surface {
  public:
    Surface() = default;

    BZ_NON_COPYABLE(Surface);

    void init(const Instance &instance, GLFWwindow &glfwWindow);
    void destroy();

    VkSurfaceKHR getHandle() const { return handle; }

  private:
    const Instance *instance;
    VkSurfaceKHR handle;
};
}