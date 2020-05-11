#include "bzpch.h"

#include "CommandPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

    constexpr uint32 ALLOCATE_BATCH_COUNT = 4;

    void CommandPool::init(const Device &device, uint32 familyIndex) {
        this->device = &device;

        nextFreeIndex = 0;
        toAllocateIndex = 0;

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = familyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        BZ_ASSERT_VK(vkCreateCommandPool(device.getHandle(), &poolInfo, nullptr, &handle));
    }

    void CommandPool::destroy() {
        vkDestroyCommandPool(device->getHandle(), handle, nullptr);
    }

    CommandBuffer& CommandPool::getCommandBuffer(QueueProperty queueProperty, bool exclusiveQueue) {
        BZ_ASSERT_CORE(nextFreeIndex < MAX_COMMAND_BUFFERS_PER_FRAME, "CommandPool has reached maximum capacity!");

        //The next free CommandBuffer is still not allocated/initialized, so allocate a batch.
        if(nextFreeIndex == toAllocateIndex) {
            uint32 toAllocateCount = std::min(ALLOCATE_BATCH_COUNT, MAX_COMMAND_BUFFERS_PER_FRAME - toAllocateIndex);

            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = handle;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = toAllocateCount;

            VkCommandBuffer newCommandBuffers[ALLOCATE_BATCH_COUNT];
            BZ_ASSERT_VK(vkAllocateCommandBuffers(device->getHandle(), &allocInfo, newCommandBuffers));

            BZ_LOG_CORE_INFO("Allocated {} CommandBuffers.", toAllocateCount);

            for(uint32 i = 0; i < toAllocateCount; ++i) {
                buffers[toAllocateIndex + i].init(newCommandBuffers[i], queueProperty, exclusiveQueue);
            }

            toAllocateIndex += toAllocateCount;
        }

        return buffers[nextFreeIndex++];
    }

    void CommandPool::reset() {
        BZ_ASSERT_VK(vkResetCommandPool(device->getHandle(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
        nextFreeIndex = 0;
    }
}