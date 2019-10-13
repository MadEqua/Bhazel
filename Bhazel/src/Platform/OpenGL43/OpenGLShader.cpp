#include "bzpch.h"

#include "OpenGLIncludes.h"

#include "Bhazel/Core/Utils.h"
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>


namespace BZ {

    static GLenum shaderTypeToGLenum(ShaderType shaderType);

    OpenGLShader::OpenGLShader(const std::string &filePath) :
        Shader(Utils::getFileNameFromPath(filePath)) {
        auto &sources = readAndPreprocessFile(filePath);
        compile(sources);
    }

    OpenGLShader::OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc) :
        Shader(name) {
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
        BZ_ASSERT_CORE(sources.size() <= 5, "Shader sources need to have at maximum 5 entries!")
        BZ_ASSERT_CORE(sources.find(ShaderType::Vertex) != sources.end() || sources.find(ShaderType::Compute) != sources.end(), "Shader code should contain at least a Vertex or Compute shader!")
        BZ_ASSERT_CORE(!(sources.find(ShaderType::Compute) != sources.end() && sources.size() > 1), "Compute shaders should be used as standalone!")

        BZ_ASSERT_GL(rendererId = glCreateProgram());

        std::array<GLuint, 5> shaderIds;

        int i = 0;
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
            shaderIds[i++] = shaderId;
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
            for(int i = 0; i < sources.size(); ++i)
                BZ_ASSERT_GL(glDeleteShader(shaderIds[i]));

            BZ_ASSERT_ALWAYS_CORE("Shader linking error:\n{0}", static_cast<char*>(infoLog.data()));
        }

        // Always detach shaders after a successful link.
        for(int i = 0; i < sources.size(); ++i)
            BZ_ASSERT_GL(glDetachShader(rendererId, shaderIds[i]));
    }

    static GLenum shaderTypeToGLenum(ShaderType shaderType) {
        switch(shaderType) {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderType::Compute:
            return GL_COMPUTE_SHADER;
        default:
            BZ_ASSERT_ALWAYS("Unknown ShaderType!");
            return 0;
        }
    }
}