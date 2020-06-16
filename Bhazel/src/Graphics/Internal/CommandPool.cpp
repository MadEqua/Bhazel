#include "bzpch.h"

#include "CommandPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

constexpr uint32 ALLOCATE_BATCH_COUNT = 4;

void CommandPool::init(const Device &device, uint32 familyIndex) {
    this->device = &device;
    this->familyIndex = familyIndex;
    nextFreeIndex = 0;
    nextToAllocateIndex = 0;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = familyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    BZ_ASSERT_VK(vkCreateCommandPool(device.getHandle(), &poolInfo, nullptr, &handle));
}

void CommandPool::destroy() {
    vkDestroyCommandPool(device->getHandle(), handle, nullptr);
}

CommandBuffer &CommandPool::getCommandBuffer() {
    BZ_ASSERT_CORE(nextFreeIndex < MAX_COMMAND_BUFFERS_PER_FRAME,
                   "Max command buffers per frame reached!");

    if (nextFreeIndex >= nextToAllocateIndex) {
        uint32 toAllocateCount =
            std::min(ALLOCATE_BATCH_COUNT, MAX_COMMAND_BUFFERS_PER_FRAME - nextToAllocateIndex);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = handle;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = toAllocateCount;

        VkCommandBuffer newCommandBuffers[ALLOCATE_BATCH_COUNT];
        BZ_ASSERT_VK(vkAllocateCommandBuffers(device->getHandle(), &allocInfo, newCommandBuffers));
        BZ_LOG_CORE_INFO("Allocated {} CommandBuffers.", toAllocateCount);

        for (uint32 i = 0; i < toAllocateCount; ++i) {
            buffers[nextFreeIndex + i].init(newCommandBuffers[i], familyIndex);
        }

        nextToAllocateIndex += toAllocateCount;
    }

    return buffers[nextFreeIndex++];
}

void CommandPool::reset() {
    BZ_ASSERT_VK(vkResetCommandPool(device->getHandle(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    nextFreeIndex = 0;
}
}