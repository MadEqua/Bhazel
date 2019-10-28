#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    class VulkanDescriptorSetLayout : public DescriptorSetLayout, public VulkanGpuObject<VkDescriptorSetLayout> {
    public:
        explicit VulkanDescriptorSetLayout(const Builder &builder);
        ~VulkanDescriptorSetLayout() override;
    };


    class VulkanDescriptorSet : public DescriptorSet, public VulkanGpuObject<VkDescriptorSet> {
    public:
        explicit VulkanDescriptorSet(const Ref<DescriptorSetLayout> &layout);

        void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) override;
    };
}
