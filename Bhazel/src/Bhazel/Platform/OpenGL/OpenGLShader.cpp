#include "bzpch.h"

#include "OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>


namespace BZ {

    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc) {

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

            BZ_LOG_CORE_ERROR("{0}", static_cast<char*>(infoLog.data()));
            BZ_ASSERT_ALWAYS_CORE("Vertex shader compilation error.");
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

            BZ_LOG_CORE_ERROR("{0}", static_cast<char*>(infoLog.data()));
            BZ_ASSERT_ALWAYS_CORE("Fragment shader compilation error.");
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

            BZ_LOG_CORE_ERROR("{0}", static_cast<char*>(infoLog.data()));
            BZ_ASSERT_ALWAYS_CORE("OpenGLShader linking error.");
            return;
        }

        // Always detach shaders after a successful link.
        glDetachShader(rendererId, vertexShader);
        glDetachShader(rendererId, fragmentShader);
    }

    OpenGLShader::~OpenGLShader() {
        glDeleteProgram(rendererId);
    }

    void OpenGLShader::bind() const {
        glUseProgram(rendererId);
    }

    void OpenGLShader::unbind() const {
        glUseProgram(0);
    }

    void OpenGLShader::setUniformInt(const std::string &name, int v) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniform1i(loc, v);
    }

    void OpenGLShader::setUniformFloat(const std::string &name, float v) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniform1f(loc, v);
    }

    void OpenGLShader::setUniformFloat2(const std::string &name, const glm::vec2 &vec) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniform2fv(loc, 1, glm::value_ptr(vec));
    }

    void OpenGLShader::setUniformFloat3(const std::string &name, const glm::vec3 &vec) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniform3fv(loc, 1, glm::value_ptr(vec));
    }

    void OpenGLShader::setUniformFloat4(const std::string &name, const glm::vec4 &vec) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniform4fv(loc, 1, glm::value_ptr(vec));
    }

    void OpenGLShader::setUniformMat3(const std::string &name, const glm::mat3 &mat) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniformMatrix3fv(loc, 1, false, glm::value_ptr(mat));
    }

    void OpenGLShader::setUniformMat4(const std::string &name, const glm::mat4 &mat) {
        GLint loc = glGetUniformLocation(rendererId, name.c_str());
        glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mat));
    }
}