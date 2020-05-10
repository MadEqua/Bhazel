#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    constexpr int MAX_SHADER_STAGE_COUNT = 6;

    struct ShaderHandles {
        VkShaderModule modules[MAX_SHADER_STAGE_COUNT];
    };

    struct ShaderStage {
        const char *path;
        VkShaderStageFlagBits stageFlag;
    };

    class Shader : public GpuObject<ShaderHandles> {
    public:
        static Ref<Shader> create(const std::initializer_list<ShaderStage> &shaderStages);

        explicit Shader(const std::initializer_list<ShaderStage> &shaderStages);
        ~Shader();

        BZ_NON_COPYABLE(Shader);

        uint32 getStageCount() const { return stageCount; }
        const ShaderStage& getStageData(uint32 stageIndex) const;
        const std::vector<std::string> getAllFilePaths() const;

        //Used with the FileWatcher for Shader hot-reloading.
        void reload();

    private:
        ShaderStage stages[MAX_SHADER_STAGE_COUNT];
        uint32 stageCount = 0;

        void init();
        void destroy();

        static std::vector<byte> readBinaryFile(const char *filePath);
        static VkShaderModule createShaderModuleFromBinaryBlob(const std::vector<byte> &binaryBlob);
    };
}