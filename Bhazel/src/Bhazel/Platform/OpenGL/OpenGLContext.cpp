#include "bzpch.h"
#include "OpenGLContext.h"
#include "OpenGLRendererAPI.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace BZ {

    static void GLAPIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity,
                                                  GLsizei length, const GLchar *message, const void *userParam) {
        const char* sourceString;
        switch(source) {
        case GL_DEBUG_SOURCE_API:
            sourceString = "API";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceString = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceString = "Window System";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceString = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceString = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceString = "Other";
            break;
        default:
            sourceString = "Unknown";
        }

        const char* typeString;
        switch(type) {
        case GL_DEBUG_TYPE_ERROR:
            typeString = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeString = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeString = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeString = "Performance";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeString = "Portablility";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeString = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeString = "Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeString = "Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeString = "Other";
            break;
        default:
            typeString = "Unknown";
        }

        switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            BZ_LOG_CORE_ERROR("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: {3}. Message: {4}", id, sourceString, typeString, "High", message);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            BZ_LOG_CORE_WARN("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: {3}. Message: {4}", id, sourceString, typeString, "Medium", message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            BZ_LOG_CORE_WARN("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: {3}. Message: {4}", id, sourceString, typeString, "Low", message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            BZ_LOG_CORE_INFO("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: {3}. Message: {4}", id, sourceString, typeString, "Notification", message);
            break;
        default:
            BZ_LOG_CORE_INFO("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: {3}. Message: {4}", id, sourceString, typeString, "Unknown", message);
        }
    }

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle) :
        windowHandle(windowHandle) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null");

        glfwMakeContextCurrent(windowHandle);
        int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        BZ_ASSERT_CORE(status, "Failed to initialize Glad!");

        BZ_LOG_CORE_INFO("OpenGL Renderer:");
        BZ_LOG_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
        BZ_LOG_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
        BZ_LOG_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
        BZ_LOG_CORE_INFO("  GLSL Version: {0}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        GLint res;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &res);

        if(res == GL_LINEAR) {
            BZ_LOG_CORE_INFO("Linear RGB Default Framebuffer (This reporting may be wrong on nVidia drivers).");

            //FIXME: This should be glDisable. But on nVidia drivers the return value is always GL_LINEAR.
            //https://devtalk.nvidia.com/default/topic/776591/opengl/gl_framebuffer_srgb-functions-incorrectly/
            //Not very bad because every system should return a srgb framebuffer anyway.
            glEnable(GL_FRAMEBUFFER_SRGB);
        }
        else if(res == GL_SRGB) {
            BZ_LOG_CORE_INFO("sRGB Default Framebuffer.");
            //enable auto Linear RGB to sRGB conversion when writing to sRGB framebuffers
            glEnable(GL_FRAMEBUFFER_SRGB);
        }

        glGetIntegerv(GL_SAMPLES, &res);
        if(res > 0) {
            BZ_LOG_CORE_INFO("Multisampled Default Framebuffer. Samples: {0}", res);
            glEnable(GL_MULTISAMPLE);
        }
        else {
            BZ_LOG_CORE_INFO("Non-Multisampled Default Framebuffer.");
        }

#ifndef BZ_DIST
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, nullptr);

        //Filter Notification messages. Disabling notifications, avoiding spam...
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#else
        glDisable(GL_DEBUG_OUTPUT);
#endif

        rendererAPI = std::make_unique<OpenGLRendererAPI>();
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void OpenGLContext::swapBuffers() {
        glfwSwapBuffers(windowHandle);
    }
}