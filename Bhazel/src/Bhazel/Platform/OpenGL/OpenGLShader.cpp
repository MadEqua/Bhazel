#include "bzpch.h"

#include "OpenGLIncludes.h"

#include "OpenGLShader.h"
#include "OpenGLBuffer.h"

#include <glm/gtc/type_ptr.hpp>


namespace BZ {

    static GLenum shaderTypeToGLenum(ShaderType shaderType);

    OpenGLShader::OpenGLShader(const std::string &filePath) {
        const std::string &source = readFile(filePath);
        auto &sources = preProcessSource(source);
        compile(sources);
    }

    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc) {
        std::unordered_map<ShaderType, std::string> sources;
        sources[ShaderType::Vertex] = vertexSrc;
        sources[ShaderType::Fragment] = fragmentSrc;
        compile(sources);
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

    void OpenGLShader::compile(const std::unordered_map<ShaderType, std::string> &sources) {
        BZ_ASSERT_CORE(sources.find(ShaderType::Vertex) != sources.end(), "Shader code should contain at least a Vertex shader!")

        BZ_ASSERT_GL(rendererId = glCreateProgram());

        std::vector<GLuint> shaderIds;
        shaderIds.reserve(sources.size());

        for(auto &kv : sources) {
            GLenum shaderType = shaderTypeToGLenum(kv.first);
            const std::string &src = kv.second;

            GLuint shaderId;
            BZ_ASSERT_GL(shaderId = glCreateShader(shaderType));
            const GLchar *source = static_cast<const GLchar *>(src.c_str());
            BZ_ASSERT_GL(glShaderSource(shaderId, 1, &source, 0));
            BZ_ASSERT_GL(glCompileShader(shaderId));

            GLint isCompiled = 0;
            BZ_ASSERT_GL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled));
            if(isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                BZ_ASSERT_GL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength));

                std::vector<GLchar> infoLog(maxLength);
                BZ_ASSERT_GL(glGetShaderInfoLog(shaderId, maxLength, &maxLength, &infoLog[0]));

                BZ_ASSERT_GL(glDeleteProgram(rendererId));
                BZ_ASSERT_GL(glDeleteShader(shaderId));

                BZ_ASSERT_ALWAYS_CORE("Shader compilation error:\n{0}", static_cast<char*>(infoLog.data()));
            }

            BZ_ASSERT_GL(glAttachShader(rendererId, shaderId));
            shaderIds.emplace_back(shaderId);
        }

        BZ_ASSERT_GL(glLinkProgram(rendererId));

        GLint isLinked = 0;
        BZ_ASSERT_GL(glGetProgramiv(rendererId, GL_LINK_STATUS, (int*) &isLinked));
        if(isLinked == GL_FALSE) {
            GLint maxLength = 0;
            BZ_ASSERT_GL(glGetProgramiv(rendererId, GL_INFO_LOG_LENGTH, &maxLength));

            std::vector<GLchar> infoLog(maxLength);
            BZ_ASSERT_GL(glGetProgramInfoLog(rendererId, maxLength, &maxLength, &infoLog[0]));

            BZ_ASSERT_GL(glDeleteProgram(rendererId));
            for(auto shaderId : shaderIds)
                BZ_ASSERT_GL(glDeleteShader(shaderId));

            BZ_LOG_CORE_ERROR("{0}", static_cast<char*>(infoLog.data()));
            BZ_ASSERT_ALWAYS_CORE("OpenGLShader linking error.");
        }

        // Always detach shaders after a successful link.
        for(auto shaderId : shaderIds)
            BZ_ASSERT_GL(glDetachShader(rendererId, shaderId));
    }

    static GLenum shaderTypeToGLenum(ShaderType shaderType) {
        switch(shaderType) {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Fragment:
            return GL_FRAGMENT_SHADER;
        default:
            BZ_ASSERT_ALWAYS("Unknown ShaderType!");
        }
    }
}