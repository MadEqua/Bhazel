#pragma once

#include "Graphics/CommandBuffer.h"
#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

class Device;

// Allocates CommandBuffers from a specific family. Internal only, not exposed to upper layers.
class CommandPool {
  public:
    CommandPool() = default;

    BZ_NON_COPYABLE(CommandPool);

    void init(const Device &device, uint32 familyIndex);
    void destroy();

    // The returned CommandBuffer is only valid until submission.
    CommandBuffer &getCommandBuffer();

    // The caller has the responsability to call when it's safe to reset the command buffers.
    void reset();

    VkCommandPool getHandle() const { return handle; }

  private:
    VkCommandPool handle;
    const Device *device;
    uint32 familyIndex;

    std::vector<CommandBuffer> buffers;
    uint32 nextFreeIndex;
};
}