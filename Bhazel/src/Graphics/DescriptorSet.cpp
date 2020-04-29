#include "bzpch.h"

#include "DescriptorSet.h"

#include "Graphics/Graphics.h"

#include "Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addDescriptorDesc(DescriptorType type, uint8 shaderStageVisibilityMask, uint32 arrayCount) {
        BZ_ASSERT_CORE(arrayCount > 0, "Array count must be > 0!");

        descriptorDescs.push_back({ type, shaderStageVisibilityMask, arrayCount });
        return *this;
    }

    Ref<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanDescriptorSetLayout>(*this);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    DescriptorSetLayout::DescriptorSetLayout(const Builder &builder) :
        descriptorDescs(std::move(builder.descriptorDescs)) {
        BZ_ASSERT_CORE(!builder.descriptorDescs.empty(), "No Descriptor descriptions added!");
    }


    Ref<DescriptorSet> DescriptorSet::create(const Ref<DescriptorSetLayout> &layout) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanDescriptorSet>(layout);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    void DescriptorSet::setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) {
        setConstantBuffers(&buffer, 1, 0, binding, &offset, &size);
    }

    void DescriptorSet::setConstantBuffers(const Ref<Buffer> buffers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 offsets[], uint32 sizes[]) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBuffer ||
            layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBufferDynamic,
            "Binding {} is not of type ConstantBuffer!", binding);
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].arrayCount >= dstArrayOffset + srcArrayCount, "Overflowing the array for binding {}!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);
        BZ_ASSERT_CORE(!buffers[0]->isDynamic() || (buffers[0]->isDynamic() && layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBufferDynamic),
            "The buffer is effectively \"dynamic\" (there are internally created replicas because of memory type), so the type on the layout needs to be DescriptorType::ConstantBufferDynamic.");

        dynamicBuffers.emplace_back(binding, buffers, srcArrayCount);
        internalSetConstantBuffer(buffers, srcArrayCount, dstArrayOffset, binding, offsets, sizes);
    }

    void DescriptorSet::setCombinedTextureSampler(const Ref<TextureView> &textureView, const Ref<Sampler> &sampler, uint32 binding) {
        setCombinedTextureSamplers(&textureView, 1, 0, sampler, binding);
    }

    void DescriptorSet::setCombinedTextureSamplers(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::CombinedTextureSampler, "Binding {} is not of type CombinedTextureSampler!", binding);
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].arrayCount >= dstArrayOffset + srcArrayCount, "Overflowing the array for binding {}!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        internalSetCombinedTextureSampler(textureViews, srcArrayCount, dstArrayOffset, sampler, binding);
    }

    void DescriptorSet::setSampledTexture(const Ref<TextureView>& textureView, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::SampledTexture, "Binding {} is not of type SampledTexture!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        internalSetSampledTexture(&textureView, 1, 0, binding);
    }

    void DescriptorSet::setSampler(const Ref<Sampler>& sampler, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::Sampler, "Binding {} is not of type Sampler!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        internalSetSampler(&sampler, 1, 0, binding);
    }

    const DescriptorSet::DynBufferData* DescriptorSet::getDynamicBufferDataByBinding(uint32 binding) const {
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);
        for(const auto &dynBufData : dynamicBuffers) {
            if(dynBufData.binding == binding) return &dynBufData;
        }
        return nullptr;
    }

    DescriptorSet::DescriptorSet(const Ref<DescriptorSetLayout> &layout) :
        layout(layout) {
    }
}