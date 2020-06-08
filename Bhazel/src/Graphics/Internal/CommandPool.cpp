#include "bzpch.h"

#include "CommandPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

constexpr uint32 ALLOCATE_BATCH_COUNT = 4;

void CommandPool::init(const Device &device, uint32 familyIndex) {
    this->device = &device;
    this->familyIndex = familyIndex;
    nextFreeIndex = 0;

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
    if (nextFreeIndex == static_cast<uint32>(buffers.size())) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = handle;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = ALLOCATE_BATCH_COUNT;

        VkCommandBuffer newCommandBuffers[ALLOCATE_BATCH_COUNT];
        BZ_ASSERT_VK(vkAllocateCommandBuffers(device->getHandle(), &allocInfo, newCommandBuffers));
        BZ_LOG_CORE_INFO("Allocated {} CommandBuffers.", ALLOCATE_BATCH_COUNT);

        buffers.resize(buffers.size() + ALLOCATE_BATCH_COUNT);
        for (uint32 i = 0; i < ALLOCATE_BATCH_COUNT; ++i) {
            buffers[nextFreeIndex + i].init(newCommandBuffers[i], familyIndex);
        }
    }

    return buffers[nextFreeIndex++];
}

void CommandPool::reset() {
    BZ_ASSERT_VK(vkResetCommandPool(device->getHandle(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    nextFreeIndex = 0;
}
}