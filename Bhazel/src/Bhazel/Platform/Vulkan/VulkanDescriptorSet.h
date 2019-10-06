#pragma once

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"
#include "Bhazel/Renderer/DescriptorSet.h"


namespace BZ {

    class VulkanDescriptorSetLayout : public DescriptorSetLayout, public VulkanGpuObject<VkDescriptorSetLayout> {
    public:
        VulkanDescriptorSetLayout(DescriptorType type, uint8 shaderStageVisibilityMask);
       ~VulkanDescriptorSetLayout() override;
    };
}
