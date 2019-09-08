#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"
#include "Bhazel/Core/Utils.h"

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

    Ref<Shader> Shader::create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::api)
        {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(name, vertexSrc, fragmentSrc);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    std::unordered_map<ShaderType, std::string> Shader::readAndPreprocessFile(const std::string & filePath) {
        
        std::ifstream in(filePath, std::ios::in);
        BZ_ASSERT_CORE(in, "Failed to load shader '{0}'.", filePath);

        const char* typeToken = "#type";
        const size_t typeTokenLength = strlen(typeToken);

        std::unordered_map<ShaderType, std::string> result;
        std::stringstream sstream;
        std::string line;
        ShaderType currentType = ShaderType::Unknown;

        while(std::getline(in, line)) {
            if(!line.empty()) {
                if(line.find(typeToken) == 0) {
                    if(currentType != ShaderType::Unknown) {
                        result[currentType] = sstream.str();
                        sstream.str("");
                        sstream.clear();
                    }
                    std::string typeString = Utils::trim(line.substr(typeTokenLength, std::string::npos));
                    currentType = shaderTypeFromString(typeString);
                }
                else if(currentType != ShaderType::Unknown) {
                    sstream << line << std::endl;
                }
            }
        }
        result[currentType] = sstream.str();
        in.close();
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


    void ShaderLibrary::add(const std::string &name, const Ref<Shader> &shader) {
        BZ_ASSERT_CORE(!exists(name), "Adding name already in use!");
        shaders[name] = shader;
    }

    void ShaderLibrary::add(const Ref<Shader> &shader) {
        add(shader->getName(), shader);
    }

    Ref<Shader> ShaderLibrary::load(const std::string &filepath) {
        auto shader = Shader::create(filepath);
        add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::load(const std::string &name, const std::string &filepath) {
        auto shader = Shader::create(filepath);
        add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::get(const std::string &name) {
        BZ_ASSERT_CORE(exists(name), "Non-existent name on shader library!");
        return shaders[name];
    }

    bool ShaderLibrary::exists(const std::string &name) const {
        return shaders.find(name) != shaders.end();
    }
}