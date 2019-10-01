#include "bzpch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Bhazel/Application.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"
#include "Bhazel/Platform/Vulkan/VulkanShader.h"
#include "Bhazel/Core/Utils.h"

#include <fstream>


namespace BZ {

    Ref<Shader> Shader::createFromSource(const std::string &filePath) {
        auto &assetsPath = Application::getInstance().getAssetsPath();
        switch(Renderer::api) {
        /*case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(assetsPath + filePath);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(assetsPath + filePath);*/
        case Renderer::API::Vulkan:
            BZ_ASSERT_ALWAYS_CORE("Not implemented!");
            return nullptr;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Shader> Shader::createFromSource(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc) {
        switch(Renderer::api) {
        /*case Renderer::API::OpenGL:
            return MakeRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Shader>(name, vertexSrc, fragmentSrc);*/
        case Renderer::API::Vulkan:
            BZ_ASSERT_ALWAYS_CORE("Not implemented!");
            return nullptr;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Shader> Shader::createFromBlob(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        switch(Renderer::api) {
            /*case Renderer::API::OpenGL:
                return MakeRef<OpenGLShader>(name, vertexBlob, fragmentBlob);
            case Renderer::API::D3D11:
                return MakeRef<D3D11Shader>(name, vertexBlob, fragmentBlob);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanShader>(name, vertexPath, fragmentPath);
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

    std::vector<char> Shader::readBlobFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        BZ_ASSERT_CORE(file, "Cannot open file {}", filePath);

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    ShaderType Shader::shaderTypeFromString(const std::string &string) {
        if(string == "Vertex" || string == "vertex")
            return ShaderType::Vertex;
        else if(string == "Fragment" || string == "fragment" || string == "Pixel" || string == "pixel")
            return ShaderType::Fragment;
        else if(string == "Compute" || string == "compute")
            return ShaderType::Compute;
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown shader type string: '{0}'", string);
            return ShaderType::Unknown;
        }
    }


    void ShaderLibrary::add(const std::string &name, const Ref<Shader> &shader) {
        BZ_ASSERT_CORE(!exists(name), "Adding name already in use!");
        shaders[name] = shader;
    }

    void ShaderLibrary::add(const Ref<Shader> &shader) {
        add(shader->getName(), shader);
    }

    Ref<Shader> ShaderLibrary::loadFromSource(const std::string &filepath) {
        auto shader = Shader::createFromSource(filepath);
        add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::loadFromSource(const std::string &name, const std::string &filepath) {
        auto shader = Shader::createFromSource(filepath);
        add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::loadFromBlob(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        auto shader = Shader::createFromBlob(name, vertexPath, fragmentPath);
        add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::get(const std::string &name) {
        BZ_ASSERT_CORE(exists(name), "A shader with name '{0}' doesn't exist on the shader library!", name)
        return shaders[name];
    }

    bool ShaderLibrary::exists(const std::string &name) const {
        return shaders.find(name) != shaders.end();
    }
}