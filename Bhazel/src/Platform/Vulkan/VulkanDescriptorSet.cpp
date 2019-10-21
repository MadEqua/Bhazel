#include "bzpch.h"

#include "VulkanDescriptorSet.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"
#include "Platform/Vulkan/VulkanBuffer.h"


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

    void VulkanDescriptorSet::setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBuffer, "Binding {} is not of type ConstantBuffer!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = static_cast<const VulkanBuffer &>(*buffer).getNativeHandle();
        bufferInfo.offset = offset;
        bufferInfo.range = size;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = nativeHandle;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(getDevice(), 1, &write, 0, nullptr);
    }
}