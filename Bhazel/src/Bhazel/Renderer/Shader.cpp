#include "bzpch.h"

#include "Shader.h"
#include <glad/glad.h>

#include <vector>

namespace BZ {

    Shader::Shader(const std::string &vertexSrc, const std::string &fragmentSrc) {

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar *source = (const GLchar *) vertexSrc.c_str();
        glShaderSource(vertexShader, 1, &source, 0);
        glCompileShader(vertexShader);

        GLint isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
            
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertexShader);

            BZ_CORE_ERROR(" {0}", static_cast<char*>(infoLog.data()));
            BZ_CORE_ASSERT(false, "Vertex shader compilation error.");
            return;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        source = (const GLchar *) fragmentSrc.c_str();
        glShaderSource(fragmentShader, 1, &source, 0);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            BZ_CORE_ERROR(" {0}", static_cast<char*>(infoLog.data()));
            BZ_CORE_ASSERT(false, "Fragment shader compilation error.");
            return;
        }

        rendererId = glCreateProgram();

        glAttachShader(rendererId, vertexShader);
        glAttachShader(rendererId, fragmentShader);

        glLinkProgram(rendererId);

        GLint isLinked = 0;
        glGetProgramiv(rendererId, GL_LINK_STATUS, (int*) &isLinked);
        if(isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(rendererId, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(rendererId, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(rendererId);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            BZ_CORE_ERROR(" {0}", static_cast<char*>(infoLog.data()));
            BZ_CORE_ASSERT(false, "Shader linking error.");
            return;
        }

        // Always detach shaders after a successful link.
        glDetachShader(rendererId, vertexShader);
        glDetachShader(rendererId, fragmentShader);
    }

    Shader::~Shader() {
        glDeleteProgram(rendererId);
    }

    void Shader::bind() const {
        glUseProgram(rendererId);
    }

    void Shader::unbind() const {
        glUseProgram(0);
    }
}