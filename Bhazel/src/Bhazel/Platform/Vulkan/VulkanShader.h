#pragma once

#include "Bhazel/Renderer/Shader.h"

#include "Bhazel/Platform/Vulkan/VulkanContext.h"
#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"


namespace BZ {

    class VulkanShader : public Shader {
    public:
        //explicit VulkanShader(const std::string &filePath);
        //VulkanShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
        VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        ~VulkanShader() override;

    private:
        VulkanContext &context;

        std::vector<VkShaderModule> shaderModules;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

        VkShaderModule createShaderModule(const std::vector<char> &codeBlob);

        friend class VulkanPipelineState;
    };
}