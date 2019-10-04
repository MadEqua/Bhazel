#include "bzpch.h"

#include "VulkanShader.h"


namespace BZ {

    VulkanShader::VulkanShader(const char *name, const std::array<std::string, SHADER_STAGES_COUNT> &codeStrings) : 
        Shader(name, codeStrings) {
        BZ_ASSERT_ALWAYS("Not implemented!");
    }

    VulkanShader::VulkanShader(const char *name, const std::array<std::vector<char>, SHADER_STAGES_COUNT> &binaryBlobs) :
        Shader(name, binaryBlobs) {

        for(int i = 0; i < SHADER_STAGES_COUNT; ++i) {
            if(stages[i])
                nativeHandle.modules[i] = createShaderModuleFromBinaryBlob(binaryBlobs[i]);
            else
                nativeHandle.modules[i] = VK_NULL_HANDLE;
        }
    }

    VulkanShader::~VulkanShader() {
        for(int i = 0; i < SHADER_STAGES_COUNT; ++i)
            vkDestroyShaderModule(getGraphicsContext().getDevice(), nativeHandle.modules[i], nullptr);
    }

    VkShaderModule VulkanShader::createShaderModuleFromBinaryBlob(const std::vector<char> &binaryBlob) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = binaryBlob.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryBlob.data());

        VkShaderModule shaderModule;
        BZ_ASSERT_VK(vkCreateShaderModule(getGraphicsContext().getDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }
}
