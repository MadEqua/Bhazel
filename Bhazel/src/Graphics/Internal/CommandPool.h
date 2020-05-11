#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/CommandBuffer.h"


namespace BZ {

    class Device;

    //Allocates CommandBuffers from a certain family. Internal only, not exposed to upper layers.
    class CommandPool {
    public:
        CommandPool() = default;

        BZ_NON_COPYABLE(CommandPool);

        void init(const Device &device, uint32 familyIndex);
        void destroy();

        //The returned CommandBuffer is only valid until submission.
        CommandBuffer& getCommandBuffer();

        //The caller has the responsability to call when it's safe to reset the command buffers.
        void reset();

        VkCommandPool getHandle() const { return handle; }

    private:
        VkCommandPool handle;
        const Device *device;

        CommandBuffer buffers[MAX_COMMAND_BUFFERS_PER_FRAME];
        uint32 nextFreeIndex;
        uint32 toAllocateIndex;
    };
}