#include "bzpch.h"

#include "OpenGLContext.h"
#include "OpenGLRendererAPI.h"

#include <GLFW/glfw3.h>
#include "OpenGLIncludes.h"


namespace BZ {

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle) :
        windowHandle(windowHandle) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null");

        glfwMakeContextCurrent(windowHandle);
        int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        BZ_ASSERT_CORE(status, "Failed to initialize Glad!");

        BZ_LOG_CORE_INFO("OpenGL Context:");
        BZ_LOG_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
        BZ_LOG_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
        BZ_LOG_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
        BZ_LOG_CORE_INFO("  GLSL Version: {0}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        GLint res;
        BZ_ASSERT_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        BZ_ASSERT_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &res));
        
        if(res == GL_LINEAR) {
            BZ_LOG_CORE_INFO("Linear RGB Default Framebuffer (This reporting may be wrong on nVidia drivers).");

            //FIXME: This should be glDisable. But on nVidia drivers the return value is always GL_LINEAR.
            //https://devtalk.nvidia.com/default/topic/776591/opengl/gl_framebuffer_srgb-functions-incorrectly/
            //Not very bad because every system should return a srgb framebuffer anyway.
            BZ_ASSERT_GL(glEnable(GL_FRAMEBUFFER_SRGB));
        }
        else if(res == GL_SRGB) {
            BZ_LOG_CORE_INFO("sRGB Default Framebuffer.");
            //enable auto Linear RGB to sRGB conversion when writing to sRGB framebuffers
            BZ_ASSERT_GL(glEnable(GL_FRAMEBUFFER_SRGB));
        }

        BZ_ASSERT_GL(glGetIntegerv(GL_SAMPLES, &res));
        if(res > 0) {
            BZ_LOG_CORE_INFO("Multisampled Default Framebuffer. Samples: {0}", res);
            BZ_ASSERT_GL(glEnable(GL_MULTISAMPLE));
        }
        else {
            BZ_LOG_CORE_INFO("Non-Multisampled Default Framebuffer.");
        }

#ifndef BZ_DIST
        BZ_ASSERT_GL(glEnable(GL_DEBUG_OUTPUT));
        BZ_ASSERT_GL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
        BZ_ASSERT_GL(glDebugMessageCallback(openglCallbackFunction, nullptr));

        //Filter Notification messages. Disabling notifications, avoiding spam...
        BZ_ASSERT_GL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE));
#else
        glDisable(GL_DEBUG_OUTPUT);
#endif

        rendererAPI = std::make_unique<OpenGLRendererAPI>();
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void OpenGLContext::swapBuffers() {
        glfwSwapBuffers(windowHandle);
    }

    void OpenGLContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);

        if(enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
    }
}