#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"


namespace BZ {

    class VulkanDevice;

    class VulkanFence {
    public:
        VulkanFence() = default;
        //VulkanFence(const VulkanDevice &device, bool signaled);
        //~VulkanFence();

        void init(const VulkanDevice &device, bool signaled);
        void destroy();

        void waitFor(uint64 timeout = 0xffffffffffffffffui64) const;
        void reset() const;

        VkFence getNativeHandle() const { return fence; }

    private:
        VkFence fence = VK_NULL_HANDLE;
        const VulkanDevice *device = nullptr;
    };


    class VulkanSemaphore {
    public:
        VulkanSemaphore() = default;
        //VulkanSemaphore(const VulkanDevice &device);
        //~VulkanSemaphore();

        void init(const VulkanDevice &device);
        void destroy();

        VkSemaphore getNativeHandle() const { return semaphore; }

    private:
        VkSemaphore semaphore = VK_NULL_HANDLE;
        const VulkanDevice *device = nullptr;
    };
}