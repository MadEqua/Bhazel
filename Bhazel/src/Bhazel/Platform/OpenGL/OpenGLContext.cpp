#include "bzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace BZ {

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle) :
        windowHandle(windowHandle) {
        BZ_CORE_ASSERT(windowHandle, "Window handle is null");
    }

    void OpenGLContext::init() {
        glfwMakeContextCurrent(windowHandle);
        int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        BZ_CORE_ASSERT(status, "Failed to initialize Glad!");
    }

    void OpenGLContext::swapBuffers() {
        glfwSwapBuffers(windowHandle);
    }
}