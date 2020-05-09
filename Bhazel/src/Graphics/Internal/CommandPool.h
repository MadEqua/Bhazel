#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Device;
    class CommandBuffer;

    //Allocates CommandBuffers from a certain family. Internal only, not exposed to upper layers.
    class CommandPool {
    public:
        CommandPool() = default;

        BZ_NON_COPYABLE(CommandPool);

        void init(const Device &device, uint32 familyIndex);
        void destroy();

        Ref<CommandBuffer> getCommandBuffer();

        //The caller has the responsability to call when it's safe to reset the command buffers.
        void reset();

        VkCommandPool getHandle() const { return handle; }

    private:
        VkCommandPool handle;
        const Device *device;

        std::vector<Ref<CommandBuffer>> buffersInUse;
        std::vector<Ref<CommandBuffer>> buffersFree;
    };
}