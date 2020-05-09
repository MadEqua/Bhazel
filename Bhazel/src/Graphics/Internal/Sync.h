#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Device;

    class Fence {
    public:
        Fence() = default;

        BZ_NON_COPYABLE(Fence);

        void init(const Device &device, bool signaled);
        void destroy();

        void waitFor(uint64 timeout = 0xffffffffffffffffui64) const;
        bool isSignaled() const;
        void reset() const;

        VkFence getHandle() const { return handle; }

    private:
        VkFence handle;
        const Device *device;
    };


    /*-------------------------------------------------------------------------------------------*/
    class Semaphore {
    public:
        Semaphore() = default;

        BZ_NON_COPYABLE(Semaphore);

        void init(const Device &device);
        void destroy();

        VkSemaphore getHandle() const { return handle; }

    private:
        VkSemaphore handle;
        const Device *device;
    };
}