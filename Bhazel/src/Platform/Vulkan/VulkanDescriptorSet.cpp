#include "bzpch.h"

#include "VulkanDescriptorSet.h"

#include "Platform/Vulkan/Internal/VulkanConversions.h"
#include "Platform/Vulkan/VulkanBuffer.h"

#include "Core/Application.h"
#include "Platform/Vulkan/VulkanContext.h"


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

        VulkanContext &context = static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext());

        //TODO: Assuming the worst case scenario and creating MAX_FRAMES_IN_FLIGHT DescriptorSets
        VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            layouts[i] = static_cast<const VulkanDescriptorSetLayout &>(*layout).getNativeHandle();

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = context.getDescriptorPool().getNativeHandle();
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = layouts;

        BZ_ASSERT_VK(vkAllocateDescriptorSets(context.getDevice(), &allocInfo, nativeHandles));
    }

    void VulkanDescriptorSet::setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) {
        BZ_ASSERT_CORE(layout->getDescriptorDescs()[binding].type == DescriptorType::ConstantBuffer, "Binding {} is not of type ConstantBuffer!", binding);
        BZ_ASSERT_CORE(binding < layout->getDescriptorDescs().size(), "Binding {} does not exist on the layout for this DescriptorSet!", binding);
        BZ_ASSERT_CORE(offset < buffer->getSize(), "Invalid offset!");
        BZ_ASSERT_CORE(size <= buffer->getSize() - offset, "Invalid size!");

        VulkanContext &context = static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext());

        VkDescriptorBufferInfo bufferInfo[MAX_FRAMES_IN_FLIGHT];
        VkWriteDescriptorSet writes[MAX_FRAMES_IN_FLIGHT];
        //TODO: Assuming the worst case scenario and writing on MAX_FRAMES_IN_FLIGHT DescriptorSets
        //uint32 writesCount = buffer->isDynamic() ? MAX_FRAMES_IN_FLIGHT : 1;

        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            uint32 baseOfReplica = buffer->isDynamic() ? i * buffer->getSize() : 0;

            bufferInfo[i].buffer = static_cast<const VulkanBuffer &>(*buffer).getNativeHandle();
            bufferInfo[i].offset = baseOfReplica + offset;
            bufferInfo[i].range = size;

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = nativeHandles[i];
            write.dstBinding = binding;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.pBufferInfo = &bufferInfo[i];

            writes[i] = write;
        }

        vkUpdateDescriptorSets(context.getDevice(), MAX_FRAMES_IN_FLIGHT, writes, 0, nullptr);
    }

    VkDescriptorSet VulkanDescriptorSet::getNativeHandle() const {
        VulkanContext &context = static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext());
        return nativeHandles[context.getCurrentFrameIndex()];
    }
}