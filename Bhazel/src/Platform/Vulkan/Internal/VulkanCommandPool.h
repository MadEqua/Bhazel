#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanQueue.h"


namespace BZ {

    class VulkanDevice;
    class QueueFamily;
    class VulkanCommandBuffer;

    //Allocates CommandBuffers from a certain family. Internal only, not exposed to upper layers.
    class VulkanCommandPool {
    public:

        void init(const VulkanDevice &device, const QueueFamily &family);
        void destroy();

        Ref<VulkanCommandBuffer> getCommandBuffer();

        //The caller has the responsability to call when it's safe to reset the command buffers.
        void reset();

        VkCommandPool getNativeHandle() const { return commandPool; }

    private:
        VkDevice device;
        VkCommandPool commandPool;

        QueueFamily family;

        std::vector<Ref<VulkanCommandBuffer>> buffersInUse;
        std::vector<Ref<VulkanCommandBuffer>> buffersFree;
    };
}