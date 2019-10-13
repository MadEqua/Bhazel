#pragma once

#include "Bhazel/Renderer/Shader.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    struct VulkanShaderNativeHandles {
        std::array<VkShaderModule, Shader::SHADER_STAGES_COUNT> modules;
    };

    class VulkanShader : public Shader, public VulkanGpuObject<VulkanShaderNativeHandles> {
    public:
        explicit VulkanShader(const Builder &builder);
        ~VulkanShader() override;

    private:
        VkShaderModule createShaderModuleFromBinaryBlob(const std::vector<char> &binaryBlob);
    };
}