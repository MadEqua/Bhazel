#include "bzpch.h"

#include "CommandPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

    constexpr uint32 ALLOCATE_BATCH_COUNT = 4;

    void CommandPool::init(const Device &device, uint32 familyIndex) {
        this->device = &device;

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = familyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        BZ_ASSERT_VK(vkCreateCommandPool(device.getHandle(), &poolInfo, nullptr, &handle));
    }

    void CommandPool::destroy() {
        vkDestroyCommandPool(device->getHandle(), handle, nullptr);
    }

    CommandBuffer& CommandPool::getCommandBuffer() {
        if(buffersFree.empty()) {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = handle;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = ALLOCATE_BATCH_COUNT;

            VkCommandBuffer commandBuffers[ALLOCATE_BATCH_COUNT];
            BZ_ASSERT_VK(vkAllocateCommandBuffers(device->getHandle(), &allocInfo, commandBuffers));

            for(uint32 i = 1; i < ALLOCATE_BATCH_COUNT; ++i) {
                buffersFree.push_back(CommandBuffer(commandBuffers[i]));
            }

            buffersInUse.push_back(CommandBuffer(commandBuffers[0]));
        }
        else {
            buffersInUse.push_back(buffersFree.back());
            buffersFree.pop_back();
        }
        return buffersInUse.back();
    }

    void CommandPool::reset() {
        std::move(buffersInUse.begin(), buffersInUse.end(), std::back_inserter(buffersFree));
        buffersInUse.clear();

        BZ_ASSERT_VK(vkResetCommandPool(device->getHandle(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    }
}