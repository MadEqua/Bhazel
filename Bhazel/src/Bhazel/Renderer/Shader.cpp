#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"

#include <fstream>


namespace BZ {

    Ref<Shader> Shader::create(const std::string &filePath) {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(filePath);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(filePath);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Shader> Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(vertexSrc, fragmentSrc);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(vertexSrc, fragmentSrc);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    std::string Shader::readFile(const std::string & filePath) {
        std::ifstream in(filePath, std::ios::in | std::ios::binary);
        BZ_ASSERT_CORE(in, "Failed to load shader '{0}'.", filePath);

        std::string result;
        in.seekg(0, std::ios::end);
        result.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&result[0], result.size());
        in.close();
        return result;
    }

    std::unordered_map<ShaderType, std::string> Shader::preProcessSource(const std::string &source) {
        BZ_ASSERT_CORE(!source.empty(), "Source is empty!");

        std::unordered_map<ShaderType, std::string> result;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);
        while(pos != std::string::npos) {
            size_t eol = source.find_first_of("\r\n", pos);
            BZ_ASSERT_CORE(eol != std::string::npos, "Shader syntax error! Missing newline after # line.");
            size_t begin = pos + typeTokenLength + 1;
            std::string typeString = source.substr(begin, eol - begin);
            ShaderType type = shaderTypeFromString(typeString);

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            pos = source.find(typeToken, nextLinePos);
            result[type] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
        }

        return result;
    }

    ShaderType Shader::shaderTypeFromString(const std::string &string) {
        if(string == "Vertex" || string == "vertex")
            return ShaderType::Vertex;
        else if(string == "Fragment" || string == "fragment" || string == "Pixel" || string == "pixel")
            return ShaderType::Fragment;
        else
            BZ_ASSERT_ALWAYS_CORE("Unknown shader type string: '{0}'", string);
    }
}