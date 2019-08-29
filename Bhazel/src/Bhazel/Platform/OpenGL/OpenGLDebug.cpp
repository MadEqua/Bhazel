#pragma once

#include <bzpch.h>
#include "OpenGLDebug.h"

#ifndef BZ_DIST

namespace BZ {

    GLenum checkOpenGLErrors() {
        GLenum ret = glGetError();
        if(ret != GL_NO_ERROR) {
            GLuint finite = 255;
            GLenum error = ret;
            while(error != GL_NO_ERROR && finite--)
                error = glGetError();

            if(error != GL_NO_ERROR) {
                BZ_LOG_CORE_ERROR("checkOpenGLErrors() failed to reset glGetError()");
            }
        }
        return ret;
    }

    void GLAPIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity,
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
            BZ_LOG_CORE_ERROR("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: High. Message: {3}", id, sourceString, typeString, message);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            BZ_LOG_CORE_WARN("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: Medium. Message: {3}", id, sourceString, typeString, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            BZ_LOG_CORE_WARN("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: Low. Message: {3}", id, sourceString, typeString, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            BZ_LOG_CORE_INFO("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: Notification. Message: {3}", id, sourceString, typeString, message);
            break;
        default:
            BZ_LOG_CORE_INFO("OpenGL Debug - Id: 0x{0:04x}. Source: {1}. Type: {2}. Severity: Unknown. Message: {3}", id, sourceString, typeString, message);
        }
    }
}
#endif