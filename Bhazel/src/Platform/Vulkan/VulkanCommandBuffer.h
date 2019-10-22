#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Graphics/CommandBuffer.h"


namespace BZ {

    class VulkanCommandBuffer : public CommandBuffer, public VulkanGpuObject<VkCommandBuffer> {
    public:
        //Allocates a CommandBuffer from the current frame pool.
        static Ref<VulkanCommandBuffer> create(QueueProperty property, bool exclusiveQueue = false);

        //FramePool is the index of the Pool to allocate the CommandBuffer from.
        //There is a Pool per each frame in flight (0 to MAX_FRAMES_IN_FLIGHT).
        //static Ref<VulkanCommandBuffer> create(QueueProperty property, uint32 framePool, bool exclusiveQueue = false);

        VulkanCommandBuffer(QueueProperty property, bool exclusiveQueue);
    };
}