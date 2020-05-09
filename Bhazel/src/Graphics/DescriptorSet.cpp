#include "bzpch.h"

#include "DescriptorSet.h"

#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"


namespace BZ {

    Ref<DescriptorSetLayout> DescriptorSetLayout::create(const std::initializer_list<DescriptorDesc> &descs) {
        return MakeRef<DescriptorSetLayout>(descs);
    }

    DescriptorSetLayout::DescriptorSetLayout(const std::initializer_list<DescriptorDesc> &descs) :
        descriptorDescs(descs) {
        BZ_ASSERT_CORE(!descriptorDescs.empty(), "No Descriptor descriptions added!");

        std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindings(descriptorDescs.size());
        for (uint32 i = 0; i < descriptorDescs.size(); ++i) {
            const auto &descriptorDesc = descriptorDescs[i];
            
            BZ_ASSERT_CORE(descriptorDesc.arrayCount > 0, "Array count must be > 0!");

            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
            descriptorSetLayoutBinding.binding = i;
            descriptorSetLayoutBinding.descriptorType = descriptorDesc.type;
            descriptorSetLayoutBinding.descriptorCount = descriptorDesc.arrayCount;
            descriptorSetLayoutBinding.stageFlags = descriptorDesc.shaderStageFlags;
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr; //TODO: For image sampling descriptors
            vkDescriptorSetLayoutBindings[i] = descriptorSetLayoutBinding;
        }

        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32>(vkDescriptorSetLayoutBindings.size());
        createInfo.pBindings = vkDescriptorSetLayoutBindings.data();

        BZ_ASSERT_VK(vkCreateDescriptorSetLayout(getVkDevice(), &createInfo, nullptr, &handle));
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(getVkDevice(), handle, nullptr);
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<DescriptorSet> DescriptorSet::create(const Ref<DescriptorSetLayout> &layout) {
        return MakeRef<DescriptorSet>(layout);
    }

    DescriptorSet::DescriptorSet(const Ref<DescriptorSetLayout> &layout) :
        layout(layout) {

        VkDescriptorSetLayout layouts [] = { layout->getHandle() };

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = getGraphicsContext().getDescriptorPool().getHandle();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        BZ_ASSERT_VK(vkAllocateDescriptorSets(getVkDevice(), &allocInfo, &handle));
    }

    void DescriptorSet::setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) {
        setConstantBuffers(&buffer, 1, 0, binding, &offset, &size);
    }

    void DescriptorSet::setConstantBuffers(const Ref<Buffer> buffers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 offsets[], uint32 sizes[]) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            "Binding {} is not of type ConstantBuffer!", binding);
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].arrayCount >= dstArrayOffset + srcArrayCount, "Overflowing the array for binding {}!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);
        BZ_ASSERT_CORE(!buffers[0]->isDynamic() || (buffers[0]->isDynamic() && layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC),
            "The buffer is effectively \"dynamic\" (there are internally created replicas because of memory type), so the type on the layout needs to be DescriptorType::ConstantBufferDynamic.");

        dynamicBuffers.emplace_back(binding, buffers, srcArrayCount);

        std::vector<VkDescriptorBufferInfo> bufferInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = buffers[i]->getHandle().bufferHandle;
            bufferInfo.offset = offsets[i];
            bufferInfo.range = sizes[i];
            bufferInfos[i] = bufferInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = handle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = layout->getDescriptorDescs()[binding].type;
        write.pBufferInfo = bufferInfos.data();
        vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorSet::setCombinedTextureSampler(const Ref<TextureView> &textureView, const Ref<Sampler> &sampler, uint32 binding) {
        setCombinedTextureSamplers(&textureView, 1, 0, sampler, binding);
    }

    void DescriptorSet::setCombinedTextureSamplers(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "Binding {} is not of type CombinedTextureSampler!", binding);
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].arrayCount >= dstArrayOffset + srcArrayCount, "Overflowing the array for binding {}!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureViews[i]->getHandle();
            imageInfo.sampler = sampler->getHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = handle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorSet::setSampledTexture(const Ref<TextureView>& textureView, uint32 binding) {
        setSampledTextures(&textureView, 1, 0, binding);
    }

    void DescriptorSet::setSampledTextures(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, "Binding {} is not of type SampledTexture!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureViews[i]->getHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = handle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);
    }

    void DescriptorSet::setSampler(const Ref<Sampler>& sampler, uint32 binding) {
        setSamplers(&sampler, 1, 0, binding);
    }

    void DescriptorSet::setSamplers(const Ref<Sampler> samplers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == VK_DESCRIPTOR_TYPE_SAMPLER, "Binding {} is not of type Sampler!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.sampler = samplers[i]->getHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = handle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getVkDevice(), 1, &write, 0, nullptr);
    }

    const DescriptorSet::DynBufferData* DescriptorSet::getDynamicBufferDataByBinding(uint32 binding) const {
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);
        for(const auto &dynBufData : dynamicBuffers) {
            if(dynBufData.binding == binding) return &dynBufData;
        }
        return nullptr;
    }

    uint32 DescriptorSet::getDynamicBufferCount() const {
        uint32 count = 0;
        for(const auto &data : dynamicBuffers) {
            count += data.arrayCount;
        }
        return count;
    }


    DescriptorSet::DynBufferData::DynBufferData(uint32 binding, const Ref<Buffer> *buffers, uint32 arrayCount) :
        binding(binding), arrayCount(arrayCount) {
        for(uint32 i = 0; i < arrayCount; ++i)
            this->buffers.push_back(buffers[i]);
    }
}