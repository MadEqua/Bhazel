#include "bzpch.h"

#include "CommandPool.h"

#include "Graphics/Internal/Device.h"

#include "Graphics/CommandBuffer.h"


namespace BZ {

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

    Ref<CommandBuffer> CommandPool::getCommandBuffer() {
        if(buffersFree.empty()) {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = handle;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            BZ_ASSERT_VK(vkAllocateCommandBuffers(device->getHandle(), &allocInfo, &commandBuffer));

            auto &ret = CommandBuffer::wrap(commandBuffer);
            buffersInUse.push_back(ret);
            return ret;
        }
        else {
            auto &freeBuffer = buffersFree.back();

            buffersInUse.push_back(freeBuffer);
            buffersFree.pop_back();

            return freeBuffer;
        }
    }

    void CommandPool::reset() {
        std::move(buffersInUse.begin(), buffersInUse.end(), std::back_inserter(buffersFree));
        buffersInUse.clear();

        BZ_ASSERT_VK(vkResetCommandPool(device->getHandle(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    }
}