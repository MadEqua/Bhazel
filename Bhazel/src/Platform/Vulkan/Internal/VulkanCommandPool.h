#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanQueue.h"


namespace BZ {

    class VulkanDevice;
    class QueueFamily;

    //Allocates CommandBuffers from a certain family. Internal only, not exposed to upper layers.
    class VulkanCommandPool {
    public:

        void init(const VulkanDevice &device, const QueueFamily &family);
        void destroy();

        void reset();

        VkCommandPool getNativeHandle() const { return commandPool; }

    private:
        VkDevice device;
        VkCommandPool commandPool;

        QueueFamily family;
    };
}