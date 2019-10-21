#include "bzpch.h"

#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/Internal/VulkanCommandPool.h"


namespace BZ {

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::create(QueueProperty property, bool exclusiveQueue) {
        return MakeRef<VulkanCommandBuffer>(property, getGraphicsContext().getCurrentFrame(), exclusiveQueue);
    }

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::create(QueueProperty property, uint32 framePool, bool exclusiveQueue) {
        return MakeRef<VulkanCommandBuffer>(property, framePool, exclusiveQueue);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(QueueProperty property, uint32 framePool, bool exclusiveQueue) {
        const auto &pool = getGraphicsContext().getCommandPool(property, framePool, exclusiveQueue);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool.getNativeHandle();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        BZ_ASSERT_VK(vkAllocateCommandBuffers(getDevice(), &allocInfo, &nativeHandle));
    }
}