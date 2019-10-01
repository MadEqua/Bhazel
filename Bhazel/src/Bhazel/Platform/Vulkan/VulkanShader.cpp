#include "bzpch.h"

#include "VulkanShader.h"

#include "Bhazel/Application.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"


namespace BZ {

    VulkanShader::VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) :
        Shader(name),
        context(static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext())) {

        shaderModules.resize(2);
        shaderStageCreateInfos.resize(2);

        shaderModules[0] = createShaderModule(readBlobFile(vertexPath));
        shaderModules[1] = createShaderModule(readBlobFile(fragmentPath));

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = shaderModules[0];
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = shaderModules[1];
        fragShaderStageInfo.pName = "main";

        shaderStageCreateInfos[0] = vertShaderStageInfo;
        shaderStageCreateInfos[1] = fragShaderStageInfo;
    }

    VulkanShader::~VulkanShader() {
    }

    VkShaderModule VulkanShader::createShaderModule(const std::vector<char> &codeBlob) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeBlob.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(codeBlob.data());

        VkShaderModule shaderModule;
        BZ_ASSERT_VK(vkCreateShaderModule(context.getDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }
}
