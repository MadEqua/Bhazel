#include "bzpch.h"

#include "VulkanCommandPool.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"


namespace BZ {

    void VulkanCommandPool::init(const VulkanDevice &device, const QueueFamily &family) {
        this->family = family;
        this->device = device.getNativeHandle();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = family.getIndex();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        BZ_ASSERT_VK(vkCreateCommandPool(this->device, &poolInfo, nullptr, &commandPool));
    }

    void VulkanCommandPool::destroy() {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    Ref<VulkanCommandBuffer> VulkanCommandPool::getCommandBuffer() {
        if(buffersFree.empty()) {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            BZ_ASSERT_VK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

            auto &ret = VulkanCommandBuffer::wrap(commandBuffer);
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

    void VulkanCommandPool::reset() {
        std::move(buffersInUse.begin(), buffersInUse.end(), std::back_inserter(buffersFree));
        buffersInUse.clear();

        BZ_ASSERT_VK(vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    }
}