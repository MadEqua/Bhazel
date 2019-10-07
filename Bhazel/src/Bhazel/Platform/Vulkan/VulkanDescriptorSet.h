#pragma once

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"
#include "Bhazel/Renderer/DescriptorSet.h"


namespace BZ {

    class VulkanDescriptorSetLayout : public DescriptorSetLayout, public VulkanGpuObject<VkDescriptorSetLayout> {
    public:
        explicit VulkanDescriptorSetLayout(const Builder &builder);
       ~VulkanDescriptorSetLayout() override;
    };


    class VulkanDescriptorSet : public DescriptorSet, public VulkanGpuObject<VkDescriptorSet> {
    public:
        static Ref<DescriptorSet> wrap(VkDescriptorSet vkDescriptorSet, const Ref<DescriptorSetLayout> &layout);

        explicit VulkanDescriptorSet(VkDescriptorSet vkDescriptorSet, const Ref<DescriptorSetLayout> &layout);
        
        void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) override;
    };


    class VulkanDescriptorPool : public DescriptorPool, public VulkanGpuObject<VkDescriptorPool> {
    public:
        explicit VulkanDescriptorPool(const Builder &builder);
        ~VulkanDescriptorPool() override;

        Ref<DescriptorSet> getDescriptorSet(const Ref<DescriptorSetLayout> &layout) const override;
    };
}
