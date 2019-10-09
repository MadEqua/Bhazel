#pragma once

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"
#include "Bhazel/Renderer/CommandBuffer.h"


namespace BZ {

    class VulkanCommandBuffer : public CommandBuffer, public VulkanGpuObject<VkCommandBuffer> {
    public:
        //Allocates a CommandBuffer from the current frame pool.
        static Ref<VulkanCommandBuffer> create(RenderQueueFamily family);

        //FramePool is the index of the Pool to allocate the CommandBuffer from.
        //There is a Pool per each frame in flight (0 to MAX_FRAMES_IN_FLIGHT).
        static Ref<VulkanCommandBuffer> create(RenderQueueFamily family, uint32 framePool);

        VulkanCommandBuffer(RenderQueueFamily family);
        VulkanCommandBuffer(RenderQueueFamily family, uint32 framePool);
    };


    //Allocates CommandBuffers
    class VulkanCommandPool : public VulkanGpuObject<VkCommandPool > {
    public:
        static Ref<VulkanCommandPool> create(RenderQueueFamily family);

        VulkanCommandPool(RenderQueueFamily family);
        ~VulkanCommandPool();

        void reset();

    private:
        RenderQueueFamily queueFamily;
    };
}