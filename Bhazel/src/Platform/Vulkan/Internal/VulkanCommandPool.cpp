#include "bzpch.h"

#include "VulkanCommandPool.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"


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

    void VulkanCommandPool::reset() {
        BZ_ASSERT_VK(vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
    }
}