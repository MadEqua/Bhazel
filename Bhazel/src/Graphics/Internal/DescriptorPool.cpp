#include "bzpch.h"

#include "DescriptorPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

void DescriptorPool::init(const Device &device, const std::initializer_list<DescriptorPoolInitData> &initDatas,
                          uint32 maxSets) {
    BZ_ASSERT_CORE(initDatas.size() > 0, "DescriptorPoolInitDatas is empty!");
    BZ_ASSERT_CORE(maxSets > 0, "maxSets needs to be greater than zero!");

    this->maxSets = maxSets;
    this->device = &device;

    nextFreeIndex = 0;

    std::vector<VkDescriptorPoolSize> vkDescriptorPoolSizes(initDatas.size());

    uint32_t i = 0;
    for (const auto &initData : initDatas) {
        vkDescriptorPoolSizes[i].type = initData.type;
        vkDescriptorPoolSizes[i].descriptorCount = initData.count;
        i++;
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = i;
    poolInfo.pPoolSizes = vkDescriptorPoolSizes.data();
    poolInfo.maxSets = maxSets;

    BZ_ASSERT_VK(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &handle));

    // Raw array to be safe of resizes, since DescriptorPool will be returning pointers to this data.
    sets = new DescriptorSet[maxSets];
}

void DescriptorPool::destroy() {
    delete[] sets;
    vkDestroyDescriptorPool(device->getHandle(), handle, nullptr);
}

DescriptorSet &DescriptorPool::getDescriptorSet(const Ref<DescriptorSetLayout> &layout) {

    BZ_ASSERT_CORE(layout, "DescriptorSetLayout is invalid!")

    // TODO: Do better than this. It's OK for now.
    BZ_ASSERT_CORE(nextFreeIndex < maxSets, "DescriptorPool has reached maximum capacity!");

    VkDescriptorSetLayout layouts[] = { layout->getHandle() };

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = handle;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    VkDescriptorSet newDescSet;
    VkResult res = vkAllocateDescriptorSets(device->getHandle(), &allocInfo, &newDescSet);

    // Besides maxSets already accounted for, the pool may exceed the number of a specific Descriptor type.
    if (res == VK_ERROR_OUT_OF_POOL_MEMORY) {
        // TODO: Do better than this. It's OK for now.
        BZ_CRITICAL_ERROR_CORE_ALWAYS("DescriptorPool has reached maximum capacity!");
    }
    else if (res < 0) {
        BZ_ASSERT_ALWAYS_CORE("vkAllocateDescriptorSets returned error {}!", res);
    }

    BZ_LOG_CORE_INFO("Allocated a DescriptorSet.");

    sets[nextFreeIndex].init(newDescSet, layout);
    return sets[nextFreeIndex++];
}

void DescriptorPool::reset() {
    BZ_ASSERT_VK(vkResetDescriptorPool(device->getHandle(), handle, 0));
    nextFreeIndex = 0;
}
}