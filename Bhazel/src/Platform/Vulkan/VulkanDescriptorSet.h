#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Graphics/DescriptorSet.h"

#include "Constants.h"


namespace BZ {

    class VulkanDescriptorSetLayout : public DescriptorSetLayout, public VulkanGpuObject<VkDescriptorSetLayout> {
    public:
        explicit VulkanDescriptorSetLayout(const Builder &builder);
       ~VulkanDescriptorSetLayout() override;
    };


    //Does not inherit from VulkanGpuObject because this class may encapsulate multiple VkDescriptorSets, if it contains a dynamic Buffer.
    class VulkanDescriptorSet : public DescriptorSet {
    public:
        explicit VulkanDescriptorSet(const Ref<DescriptorSetLayout> &layout);
        
        void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) override;

        VkDescriptorSet getNativeHandle() const;

    private:
        VkDescriptorSet nativeHandles[MAX_FRAMES_IN_FLIGHT];
    };
}
