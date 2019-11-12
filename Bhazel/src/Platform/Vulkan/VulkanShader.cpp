#include "bzpch.h"

#include "VulkanShader.h"


namespace BZ {

    VulkanShader::VulkanShader(const Builder &builder) :
        Shader(builder) {

        if(builder.useBinaryBlob) {
            for(int i = 0; i < SHADER_STAGES_COUNT; ++i) {
                if(stages[i])
                    nativeHandle.modules[i] = createShaderModuleFromBinaryBlob(builder.binaryBlobs[i]);
                else
                    nativeHandle.modules[i] = VK_NULL_HANDLE;
            }
        }
        else {
            BZ_ASSERT_ALWAYS("Not implemented!");
        }
    }

    VulkanShader::~VulkanShader() {
        for(int i = 0; i < SHADER_STAGES_COUNT; ++i)
            vkDestroyShaderModule(getDevice(), nativeHandle.modules[i], nullptr);
    }

    VkShaderModule VulkanShader::createShaderModuleFromBinaryBlob(const std::vector<char> &binaryBlob) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = binaryBlob.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryBlob.data());

        VkShaderModule shaderModule;
        BZ_ASSERT_VK(vkCreateShaderModule(getDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }
}
