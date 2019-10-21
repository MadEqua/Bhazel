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


    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::addDescriptorTypeCount(DescriptorType type, uint32 count) {
        countPerType[static_cast<int>(type)] += count;
        totalCount += count;
        return *this;
    }

    Ref<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const {
        return MakeRef<VulkanDescriptorPool>(*this);
    }

    VulkanDescriptorPool::VulkanDescriptorPool(const Builder &builder) {
        BZ_ASSERT_CORE(builder.totalCount > 0, "DescriptorPool is empty!");

        const int typeCount = static_cast<int>(DescriptorType::Count);
        VkDescriptorPoolSize poolSizeForType[typeCount];
        
        uint32_t usedSlots = 0;
        for(uint32_t i = 0; i < typeCount; ++i) {
            DescriptorType type = static_cast<DescriptorType>(i);

            if(builder.countPerType[i]) {
                poolSizeForType[usedSlots].type = descriptorTypeToVk(type);
                poolSizeForType[usedSlots].descriptorCount = builder.countPerType[i];
                usedSlots++;
            }
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = 0;
        poolInfo.poolSizeCount = usedSlots;
        poolInfo.pPoolSizes = poolSizeForType;
        poolInfo.maxSets = builder.totalCount;

        BZ_ASSERT_VK(vkCreateDescriptorPool(getDevice(), &poolInfo, nullptr, &nativeHandle));
    }

    VulkanDescriptorPool::~VulkanDescriptorPool() {
        vkDestroyDescriptorPool(getDevice(), nativeHandle, nullptr);
    }

    void VulkanDescriptorPool::reset() {
        BZ_ASSERT_VK(vkResetDescriptorPool(getDevice(), nativeHandle, 0));
    }
}