#include "bzpch.h"

#include "DescriptorSet.h"

#include "Graphics/Graphics.h"

#include "Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addDescriptorDesc(DescriptorType type, uint8 shaderStageVisibilityMask, uint32 arrayCount) {
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

    DescriptorSet::DescriptorSet(const Ref<DescriptorSetLayout> &layout) :
        layout(layout) {
    }
}