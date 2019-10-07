#include "bzpch.h"

#include "Shader.h"

#include "Bhazel/Renderer/Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"
#include "Bhazel/Platform/D3D11/D3D11Shader.h"
#include "Bhazel/Platform/Vulkan/VulkanShader.h"
#include "Bhazel/Core/Utils.h"

#include <fstream>


namespace BZ {

    Shader::Builder& Shader::Builder::fromSingleSourceFile(const char *filePath) {
        BZ_ASSERT_CORE(!useBinaryBlob.has_value() || !useBinaryBlob.value(), "Shader is already using binary data!");

        std::string path = Application::getInstance().getAssetsPath() + filePath;
        readAndPreprocessSingleSourceFile(path.c_str(), codeStrings);
        useBinaryBlob = false;
        return *this;
    }

    Shader::Builder& Shader::Builder::fromString(ShaderStage type, const char *code) {
        BZ_ASSERT_CORE(!useBinaryBlob.has_value() || !useBinaryBlob.value(), "Shader is already using binary data!");

        codeStrings[static_cast<int>(type)] = code;
        useBinaryBlob = false;
        return *this;
    }

    Shader::Builder& Shader::Builder::fromBinaryFile(ShaderStage type, const char *filePath) {
        BZ_ASSERT_CORE(!useBinaryBlob.has_value() || useBinaryBlob.value(), "Shader is already using text data!")

        std::string path = Application::getInstance().getAssetsPath() + filePath;
        binaryBlobs[static_cast<int>(type)] = readBinaryFile(path.c_str());
        useBinaryBlob = true;
        return *this;
    }

    Shader::Builder& Shader::Builder::fromSourceFile(ShaderStage type, const char *filePath) {
        BZ_ASSERT_CORE(!useBinaryBlob.has_value() || !useBinaryBlob.value(), "Shader is already using binary data!");

        std::string path = Application::getInstance().getAssetsPath() + filePath;
        codeStrings[static_cast<int>(type)] = readSourceFile(path.c_str());
        useBinaryBlob = false;
        return *this;
    }

    Shader::Builder& Shader::Builder::setName(const char *name) {
        this->name = name;
        return *this;
    }

    Ref<Shader> Shader::Builder::build() const {
        switch(Renderer::api) {
            /*case Renderer::API::OpenGL:
                return MakeRef<OpenGLTexture2D>(assetsPath + path);
            case Renderer::API::D3D11:
                return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanShader>(*this);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    void Shader::Builder::readAndPreprocessSingleSourceFile(const char *filePath, std::array<std::string, SHADER_STAGES_COUNT> &out) {
        std::ifstream file(filePath, std::ios::in);
        BZ_ASSERT_CORE(file, "Failed to load file '{}'!", filePath);

        const char *typeToken = "#type";
        const size_t typeTokenLength = strlen(typeToken);

        std::stringstream sstream;
        std::string line;
        std::optional<ShaderStage> currentType;

        while(std::getline(file, line)) {
            if(!line.empty()) {
                if(line.find(typeToken) == 0) {
                    if(currentType.has_value()) {
                        out[static_cast<int>(*currentType)] = sstream.str();
                        sstream.str("");
                        sstream.clear();
                    }
                    std::string typeString = Utils::trim(line.substr(typeTokenLength, std::string::npos));
                    currentType = shaderTypeFromString(typeString);
                }
                else if(currentType.has_value()) {
                    sstream << line << std::endl;
                }
            }
        }
        out[static_cast<int>(*currentType)] = sstream.str();
        file.close();
    }

    std::vector<char> Shader::Builder::readBinaryFile(const char *filePath) {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        BZ_ASSERT_CORE(file, "Failed to load file '{}'!", filePath);

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    std::string Shader::Builder::readSourceFile(const char *filePath) {
        std::ifstream file(filePath, std::ios::in);
        BZ_ASSERT_CORE(file, "Failed to load file '{}'!", filePath);
        
        size_t fileSize = (size_t)file.tellg();
        std::string content;
        content.reserve(fileSize);
        file.seekg(0);
        content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        file.close();
        return content;
    }

    ShaderStage Shader::Builder::shaderTypeFromString(const std::string &string) {
        if(string == "Vertex" || string == "vertex")
            return ShaderStage::Vertex;
        else if(string == "TesselationEvaluation" || string == "TE")
            return ShaderStage::TesselationEvaluation;
        else if(string == "TesselationControl" || string == "TC")
            return ShaderStage::TesselationControl;
        else if(string == "Geometry" || string == "geometry")
            return ShaderStage::Geometry;
        else if(string == "Fragment" || string == "fragment" || string == "Pixel" || string == "pixel")
            return ShaderStage::Fragment;
        else if(string == "Compute" || string == "compute")
            return ShaderStage::Compute;
        else {
            BZ_ASSERT_ALWAYS_CORE("Unknown shader type string: '{}'", string);
            return ShaderStage::Vertex;
        }
    }


    Shader::Shader(const Builder &builder) :
        name(builder.name) {

        BZ_ASSERT_CORE(builder.useBinaryBlob.has_value(), "No shader data was set on the Builder!");
        BZ_ASSERT_CORE(builder.name, "Shader needs a name!");

        if(builder.useBinaryBlob) {
            for(int i = 0; i < SHADER_STAGES_COUNT; ++i) {
                stages[i] = !builder.binaryBlobs[i].empty();
            }
        }
        else {
            for(int i = 0; i < SHADER_STAGES_COUNT; ++i) {
                stages[i] = !builder.codeStrings[i].empty();
            }
        }
    }

    uint32 Shader::getStageCount() const {
        uint32 count = 0;
        for(int i = 0; i < SHADER_STAGES_COUNT; ++i)
            if(stages[i]) count++;
        return count;
    }

    bool Shader::isStagePresent(ShaderStage stage) const {
        return stages[static_cast<int>(stage)];
    }



    void ShaderLibrary::add(const char *name, const Ref<Shader> &shader) {
        BZ_ASSERT_CORE(!exists(name), "Adding name already in use!");
        shaders[name] = shader;
    }

    void ShaderLibrary::add(const Ref<Shader> &shader) {
        add(shader->getName().c_str(), shader);
    }

    Ref<Shader> ShaderLibrary::get(const char *name) {
        BZ_ASSERT_CORE(exists(name), "A shader with name '{}' doesn't exist on the shader library!", name)
        return shaders[name];
    }

    bool ShaderLibrary::exists(const char *name) const {
        return shaders.find(name) != shaders.end();
    }
}