#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Graphics/CommandBuffer.h"


namespace BZ {

    class VulkanCommandBuffer : public CommandBuffer, public VulkanGpuObject<VkCommandBuffer> {
    public:
        static Ref<VulkanCommandBuffer> wrap(VkCommandBuffer vkCommandBuffer);

        VulkanCommandBuffer(VkCommandBuffer vkCommandBuffer);
    };
}