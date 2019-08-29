#pragma once

#include <glad/glad.h>


#ifndef BZ_DIST

namespace BZ {
    GLenum checkOpenGLErrors();

    void GLAPIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar *message, const void *userParam);
}

#define BZ_ASSERT_GL(call) { \
    (call); \
    GLenum err = checkOpenGLErrors(); \
    if(err != GL_NO_ERROR) \
        BZ_ASSERT_ALWAYS_CORE("OpenGL Error. Code: 0x{0:04x}.", err); \
}

#define BZ_LOG_GL(call) { \
    (call); \
    GLenum err = checkOpenGLErrors(); \
    if(err != GL_NO_ERROR) \
        BZ_LOG_CORE_ERROR("OpenGL Error. Code: 0x{0:04x}. File: {1}. Line: {2}.", err, __FILE__, __LINE__); \
}

#else
#define BZ_ASSERT_GL(call) call
#define BZ_LOG_GL(call) call
#endif
