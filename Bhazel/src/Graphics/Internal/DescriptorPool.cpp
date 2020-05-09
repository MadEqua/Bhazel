#include "bzpch.h"

#include "DescriptorPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

    void DescriptorPool::init(const Device &device, const std::initializer_list<DescriptorPoolInitData> &initDatas, uint32 maxSets) {
        BZ_ASSERT_CORE(initDatas.size() > 0, "DescriptorPoolInitDatas is empty!");

        this->device = &device;

        std::vector<VkDescriptorPoolSize> vkDescriptorPoolSizes(initDatas.size());

        uint32_t i = 0;
        for(const auto &initData : initDatas) {
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
    }

    void DescriptorPool::destroy() {
        vkDestroyDescriptorPool(device->getHandle(), handle, nullptr);
    }

    void DescriptorPool::reset() {
        BZ_ASSERT_VK(vkResetDescriptorPool(device->getHandle(), handle, 0));
    }
}