#include "bzpch.h"

#include "DescriptorSet.h"

#include "Bhazel/Renderer/Renderer.h"

#include "Bhazel/Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addDescriptorDesc(DescriptorType type, uint8 shaderStageVisibilityMask, uint32 arrayCount) {
        BZ_ASSERT_CORE(arrayCount > 0, "Array count must be > 0!");

        descriptorDescs.push_back({ type, shaderStageVisibilityMask, arrayCount });
        return *this;
    }

    Ref<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        switch(Renderer::api) {
        case Renderer::API::Vulkan:
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


    DescriptorSet::DescriptorSet(const Ref<DescriptorSetLayout> &layout) : 
        layout(layout) {
    }


    DescriptorPool::Builder& DescriptorPool::Builder::addDescriptorTypeCount(DescriptorType type, uint32 count) {
        countPerType[static_cast<int>(type)] += count;
        totalCount += count;
        return *this;
    }

    Ref<DescriptorPool> DescriptorPool::Builder::build() const {
        switch(Renderer::api) {
        case Renderer::API::Vulkan:
            return MakeRef<VulkanDescriptorPool>(*this);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    DescriptorPool::DescriptorPool(const Builder &builder) {
        BZ_ASSERT_CORE(builder.totalCount > 0, "DescriptorPool is empty!");
    }
}