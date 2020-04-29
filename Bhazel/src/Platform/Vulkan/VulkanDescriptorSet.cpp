#include "bzpch.h"

#include "VulkanDescriptorSet.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"


namespace BZ {

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const Builder &builder) :
        DescriptorSetLayout(builder) {

        std::vector<VkDescriptorSetLayoutBinding> vkDescriptorSetLayoutBindings(builder.descriptorDescs.size());
        for(int i = 0; i < builder.descriptorDescs.size(); ++i) {
            const auto &descriptorDesc = builder.descriptorDescs[i];

            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
            descriptorSetLayoutBinding.binding = i;
            descriptorSetLayoutBinding.descriptorType = descriptorTypeToVk(descriptorDesc.type);
            descriptorSetLayoutBinding.descriptorCount = descriptorDesc.arrayCount;
            descriptorSetLayoutBinding.stageFlags = shaderStageMaskToVk(builder.descriptorDescs[i].shaderStageVisibililtyMask);
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr; //TODO: For image sampling descriptors
            vkDescriptorSetLayoutBindings[i] = descriptorSetLayoutBinding;
        }

        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32>(vkDescriptorSetLayoutBindings.size());
        createInfo.pBindings = vkDescriptorSetLayoutBindings.data();

        BZ_ASSERT_VK(vkCreateDescriptorSetLayout(getDevice(), &createInfo, nullptr, &nativeHandle));
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(getDevice(), nativeHandle, nullptr);
    }


    VulkanDescriptorSet::VulkanDescriptorSet(const Ref<DescriptorSetLayout> &layout) :
        DescriptorSet(layout) {

        VkDescriptorSetLayout layouts[] = { static_cast<const VulkanDescriptorSetLayout &>(*layout).getNativeHandle() };

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = getGraphicsContext().getDescriptorPool().getNativeHandle();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        BZ_ASSERT_VK(vkAllocateDescriptorSets(getDevice(), &allocInfo, &nativeHandle));
    }

    void VulkanDescriptorSet::internalSetConstantBuffer(const Ref<Buffer> *buffers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 *offsets, uint32 *sizes) {
        std::vector<VkDescriptorBufferInfo> bufferInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = static_cast<const VulkanBuffer &>(*buffers[i]).getNativeHandle();
            bufferInfo.offset = offsets[i];
            bufferInfo.range = sizes[i];
            bufferInfos[i] = bufferInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = nativeHandle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = descriptorTypeToVk(layout->getDescriptorDescs()[binding].type);
        write.pBufferInfo = bufferInfos.data();
        vkUpdateDescriptorSets(getDevice(), 1, &write, 0, nullptr);
    }

    void VulkanDescriptorSet::internalSetCombinedTextureSampler(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding) {
        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = static_cast<const VulkanTextureView &>(*textureViews[i]).getNativeHandle();
            imageInfo.sampler = static_cast<const VulkanSampler &>(*sampler).getNativeHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = nativeHandle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getDevice(), 1, &write, 0, nullptr);
    }

    void VulkanDescriptorSet::internalSetSampledTexture(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) {
        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = static_cast<const VulkanTextureView &>(*textureViews[i]).getNativeHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = nativeHandle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getDevice(), 1, &write, 0, nullptr);
    }

    void VulkanDescriptorSet::internalSetSampler(const Ref<Sampler> *samplers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) {
        std::vector<VkDescriptorImageInfo> imageInfos(srcArrayCount);
        for (uint32 i = 0; i < srcArrayCount; ++i) {
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.sampler = static_cast<const VulkanSampler&>(*samplers[i]).getNativeHandle();
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = nativeHandle;
        write.dstBinding = binding;
        write.dstArrayElement = dstArrayOffset;
        write.descriptorCount = srcArrayCount;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        write.pImageInfo = imageInfos.data();
        vkUpdateDescriptorSets(getDevice(), 1, &write, 0, nullptr);
    }
}