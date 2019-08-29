#include "bzpch.h"

#include "OpenGLShader.h"
#include "OpenGLBuffer.h"

#include "OpenGLIncludes.h"
#include <glm/gtc/type_ptr.hpp>


namespace BZ {

    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc) {

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar *source = (const GLchar *) vertexSrc.c_str();
        BZ_ASSERT_GL(glShaderSource(vertexShader, 1, &source, 0));
        BZ_ASSERT_GL(glCompileShader(vertexShader));

        GLint isCompiled = 0;
        BZ_ASSERT_GL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled));
        if(isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            BZ_ASSERT_GL(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength));

            std::vector<GLchar> infoLog(maxLength);
            BZ_ASSERT_GL(glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]));

            BZ_ASSERT_GL(glDeleteShader(vertexShader));

            BZ_ASSERT_ALWAYS_CORE("Vertex shader compilation error:\n{0}", static_cast<char*>(infoLog.data()));
            return;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        source = (const GLchar *) fragmentSrc.c_str();
        BZ_ASSERT_GL(glShaderSource(fragmentShader, 1, &source, 0));
        BZ_ASSERT_GL(glCompileShader(fragmentShader));

        BZ_ASSERT_GL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled));
        if(isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            BZ_ASSERT_GL(glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength));

            std::vector<GLchar> infoLog(maxLength);
            BZ_ASSERT_GL(glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]));

            BZ_ASSERT_GL(glDeleteShader(fragmentShader));
            BZ_ASSERT_GL(glDeleteShader(vertexShader));

            BZ_ASSERT_ALWAYS_CORE("Fragment shader compilation error:\n{0}", static_cast<char*>(infoLog.data()));

            return;
        }

        rendererId = glCreateProgram();

        BZ_ASSERT_GL(glAttachShader(rendererId, vertexShader));
        BZ_ASSERT_GL(glAttachShader(rendererId, fragmentShader));

        BZ_ASSERT_GL(glLinkProgram(rendererId));

        GLint isLinked = 0;
        BZ_ASSERT_GL(glGetProgramiv(rendererId, GL_LINK_STATUS, (int*) &isLinked));
        if(isLinked == GL_FALSE) {
            GLint maxLength = 0;
            BZ_ASSERT_GL(glGetProgramiv(rendererId, GL_INFO_LOG_LENGTH, &maxLength));

            std::vector<GLchar> infoLog(maxLength);
            BZ_ASSERT_GL(glGetProgramInfoLog(rendererId, maxLength, &maxLength, &infoLog[0]));

            BZ_ASSERT_GL(glDeleteProgram(rendererId));
            BZ_ASSERT_GL(glDeleteShader(vertexShader));
            BZ_ASSERT_GL(glDeleteShader(fragmentShader));

            BZ_LOG_CORE_ERROR("{0}", static_cast<char*>(infoLog.data()));
            BZ_ASSERT_ALWAYS_CORE("OpenGLShader linking error.");
            return;
        }

        // Always detach shaders after a successful link.
        BZ_ASSERT_GL(glDetachShader(rendererId, vertexShader));
        BZ_ASSERT_GL(glDetachShader(rendererId, fragmentShader));
    }

    OpenGLShader::~OpenGLShader() {
        BZ_ASSERT_GL(glDeleteProgram(rendererId));
    }

    void OpenGLShader::bindToPipeline() const {
        BZ_ASSERT_GL(glUseProgram(rendererId));
    }

    void OpenGLShader::unbindFromPipeline() const {
        BZ_ASSERT_GL(glUseProgram(0));
    }
}