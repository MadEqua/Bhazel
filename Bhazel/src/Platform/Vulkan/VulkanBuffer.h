#include "bzpch.h"

#include "Graphics/Buffer.h"

#include "Platform/Vulkan/VulkanIncludes.h"


namespace BZ {

    struct VulkanBufferHandlesContainer {
        VkBuffer bufferHandle;
        VkDeviceMemory memoryHandle;
    };

    class VulkanBuffer : public Buffer<VulkanBufferHandlesContainer> {
    public:
        VulkanBuffer(BufferType type, uint32 size, const void *data, const DataLayout &layout);
    };
}