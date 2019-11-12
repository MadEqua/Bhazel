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
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBuffer ||
                       layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBufferDynamic,
                       "Binding {} is not of type ConstantBuffer!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        if(buffer->isDynamic()) {
            //If the buffer is "dynamic" then the engine will internally create replicas for each frame in flight, so we need a dynamic descriptor set behind the scenes.
            if(layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBuffer) {
                layout->getDescriptorDescs()[binding].type = DescriptorType::ConstantBufferDynamic;
                dynamicBuffers.emplace_back(binding, buffer, true);
            }
            else {
                dynamicBuffers.emplace_back(binding, buffer, false);
            }
        }

        internalSetConstantBuffer(buffer, binding, offset, size);
    }

    void DescriptorSet::setCombinedTextureSampler(const Ref<TextureView> &textureView, const Ref<Sampler> &sampler, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::CombinedTextureSampler, "Binding {} is not of type CombinedTextureSampler!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        internalSetCombinedTextureSampler(textureView, sampler, binding);
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