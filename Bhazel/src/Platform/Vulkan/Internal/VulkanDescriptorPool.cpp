#include "bzpch.h"

#include "VulkanDescriptorPool.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"


namespace BZ {

    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::addDescriptorTypeCount(DescriptorType type, uint32 count) {
        countPerType[static_cast<int>(type)] += count;
        totalCount += count;
        return *this;
    }

    void VulkanDescriptorPool::init(const VulkanDevice &device, const Builder &builder) {
        BZ_ASSERT_CORE(builder.totalCount > 0, "DescriptorPool is empty!");

        this->device = device.getNativeHandle();

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

        BZ_ASSERT_VK(vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &descriptorPool));
    }

    void VulkanDescriptorPool::destroy() {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }

    void VulkanDescriptorPool::reset() {
        BZ_ASSERT_VK(vkResetDescriptorPool(device, descriptorPool, 0));
    }
}