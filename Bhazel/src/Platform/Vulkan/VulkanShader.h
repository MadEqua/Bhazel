#pragma once

#include "Graphics/Shader.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    struct VulkanShaderNativeHandles {
        std::array<VkShaderModule, Shader::SHADER_STAGES_COUNT> modules;
    };

    class VulkanShader : public Shader, public VulkanGpuObject<VulkanShaderNativeHandles> {
    public:
        explicit VulkanShader(const Builder &builder);
        ~VulkanShader() override;

    private:
        void init(const Builder &builder) override;
        void destroy() override;

        VkShaderModule createShaderModuleFromBinaryBlob(const std::vector<char> &binaryBlob);
    };
}