#include "bzpch.h"

#include "VulkanCommandBuffer.h"


namespace BZ {

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::create(RenderQueueFamily family) {
        return MakeRef<VulkanCommandBuffer>(family);
    }

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::create(RenderQueueFamily family, uint32 framePool) {
        return MakeRef<VulkanCommandBuffer>(family, framePool);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(RenderQueueFamily family) :
        VulkanCommandBuffer(family, getGraphicsContext().getCurrentFrame()) {
    }

    VulkanCommandBuffer::VulkanCommandBuffer(RenderQueueFamily family, uint32 framePool) {
        const auto &pool = getGraphicsContext().getCommandPool(family, framePool);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool.getNativeHandle();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        BZ_ASSERT_VK(vkAllocateCommandBuffers(getDevice(), &allocInfo, &nativeHandle));
    }


    Ref<VulkanCommandPool> VulkanCommandPool::create(RenderQueueFamily family) {
        return MakeRef<VulkanCommandPool>(family);
    }

    VulkanCommandPool::VulkanCommandPool(RenderQueueFamily family) :
        queueFamily(family) {

        const auto &queueFamilyIndices = getGraphicsContext().getQueueFamilyIndices();
        uint32_t index = -1;
        if(family == RenderQueueFamily::Graphics) 
            index = queueFamilyIndices.graphicsFamily.value();
        else if(family == RenderQueueFamily::Compute) 
            index = queueFamilyIndices.computeFamily.value();
        else if(family == RenderQueueFamily::Transfer)
            index = queueFamilyIndices.transferFamily.value();
        else if(family == RenderQueueFamily::Present)
            index = queueFamilyIndices.presentFamily.value();

        BZ_ASSERT_CORE(index >= 0, "Invalid queue family index!");

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = index;
        poolInfo.flags = 0;

        BZ_ASSERT_VK(vkCreateCommandPool(getDevice(), &poolInfo, nullptr, &nativeHandle));
    }

    VulkanCommandPool::~VulkanCommandPool() {
        vkDestroyCommandPool(getDevice(), nativeHandle, nullptr);
    }
}