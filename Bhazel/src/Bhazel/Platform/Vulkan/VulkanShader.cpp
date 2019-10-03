#include "bzpch.h"

#include "VulkanShader.h"


namespace BZ {

    VulkanShader::VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) :
        Shader(name) {

        nativeHandle.vertexModule = createShaderModule(readBlobFile(vertexPath));
        nativeHandle.fragmentModule = createShaderModule(readBlobFile(fragmentPath));
    }

    VulkanShader::~VulkanShader() {
        vkDestroyShaderModule(getGraphicsContext().getDevice(), nativeHandle.vertexModule, nullptr);
        vkDestroyShaderModule(getGraphicsContext().getDevice(), nativeHandle.fragmentModule, nullptr);
    }

    VkShaderModule VulkanShader::createShaderModule(const std::vector<char> &codeBlob) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeBlob.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(codeBlob.data());

        VkShaderModule shaderModule;
        BZ_ASSERT_VK(vkCreateShaderModule(getGraphicsContext().getDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }
}
