#include "bzpch.h"

#include "Shader.h"

#include <fstream>


namespace BZ {

    Ref<Shader> Shader::create(const std::initializer_list<ShaderStage>& shaderStages) {
        return MakeRef<Shader>(shaderStages);
    }

    Shader::Shader(const std::initializer_list<ShaderStage>& shaderStages) {
        BZ_ASSERT_CORE(shaderStages.size() > 0, "Can't create a Shader with no Stages!");

        for(auto &stage : shaderStages) {
            stages[stageCount++] = stage;
        }

        init();
    }

    Shader::~Shader() {
        for (uint32 i = 0; i < stageCount; ++i)
            vkDestroyShaderModule(getVkDevice(), handle.modules[i], nullptr);
    }

    void Shader::reload() {
        destroy();
        init();
    }

    void Shader::init() {
        auto &assetsPath = Application::get().getAssetsPath();
        for (uint32 i = 0; i < stageCount; ++i) {
            handle.modules[i] = createShaderModuleFromBinaryBlob(readBinaryFile((assetsPath  + stages[i].path).c_str()));
        }
    }

    void Shader::destroy() {
        for (uint32 i = 0; i < stageCount; ++i)
            vkDestroyShaderModule(getVkDevice(), handle.modules[i], nullptr);
    }

    const ShaderStage &Shader::getStageData(uint32 stageIndex) const {
        BZ_ASSERT_CORE(stageIndex < stageCount, "Invalid index!");
        return stages[stageIndex];
    }

    VkShaderModule Shader::createShaderModuleFromBinaryBlob(const std::vector<byte> &binaryBlob) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = binaryBlob.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryBlob.data());

        VkShaderModule shaderModule;
        BZ_ASSERT_VK(vkCreateShaderModule(getVkDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    const std::vector<std::string> Shader::getAllFilePaths() const {
        std::vector<std::string> ret;
        for (uint32 i = 0; i < stageCount; ++i) {
            ret.push_back(stages[i].path);
        }
        return ret;
    }

    std::vector<byte> Shader::readBinaryFile(const char *filePath) {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        BZ_CRITICAL_ERROR_CORE(file, "Failed to load file '{}'!", filePath);

        size_t fileSize = (size_t)file.tellg();
        std::vector<byte> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        return buffer;
    }
}