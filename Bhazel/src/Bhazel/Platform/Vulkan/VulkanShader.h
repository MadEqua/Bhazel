#pragma once

#include "Bhazel/Renderer/Shader.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    struct VulkanShaderNativeHandles {
        VkShaderModule vertexModule;
        VkShaderModule fragmentModule;
    };

    class VulkanShader : public Shader, public VulkanGpuObject<VulkanShaderNativeHandles> {
    public:
        //explicit VulkanShader(const std::string &filePath);
        //VulkanShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
        VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        ~VulkanShader() override;

    private:
        struct CreateInfos {
            VkPipelineShaderStageCreateInfo vertexCreateInfo;
            VkPipelineShaderStageCreateInfo fragmentCreateInfo;
        };
        CreateInfos createInfos;

        VkShaderModule createShaderModule(const std::vector<char> &codeBlob);
    };
}