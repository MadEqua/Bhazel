#include "bzpch.h"

#include "VulkanCommandBuffer.h"


namespace BZ {

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::wrap(VkCommandBuffer vkCommandBuffer) {
        return MakeRef<VulkanCommandBuffer>(vkCommandBuffer);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer vkCommandBuffer) {
        nativeHandle = vkCommandBuffer;
    }
}