#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    class Device;

    class Fence : public GpuObject<VkFence> {
    public:
        static Ref<Fence> create(bool signaled);

        Fence(bool signaled);
        ~Fence();

        BZ_NON_COPYABLE(Fence);

        void waitFor(uint64 timeout = 0xffffffffffffffffui64) const;
        bool isSignaled() const;
        void reset() const;
    };


    /*-------------------------------------------------------------------------------------------*/
    class Semaphore : public GpuObject<VkSemaphore> {
    public:
        static Ref<Semaphore> create();

        Semaphore();
        ~Semaphore();

        BZ_NON_COPYABLE(Semaphore);
    };
}