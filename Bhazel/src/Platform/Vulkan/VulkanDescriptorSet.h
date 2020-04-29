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

    private:
        void internalSetConstantBuffer(const Ref<Buffer> *buffers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 *offsets, uint32 *sizes) override;
        void internalSetCombinedTextureSampler(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding) override;
        void internalSetSampledTexture(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) override;
        void internalSetSampler(const Ref<Sampler> *samplers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) override;
    };
}
