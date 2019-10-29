#pragma once

#include "Graphics/Buffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"

#include <vk_mem_alloc.h>


namespace BZ {

    class VulkanContext;

    class VulkanBuffer : public Buffer, public VulkanGpuObject<VkBuffer> {
    public:
        VulkanBuffer(BufferType type, uint32 size, MemoryType memoryType, const DataLayout *layout);
        virtual ~VulkanBuffer() override;

        void internalSetData(const void *data, uint32 offset, uint32 size) override;
        byte* internalMap(uint32 offset, uint32 size) override;
        void internalUnmap() override;

    private:
        VmaAllocation allocationHandle;

        VkBuffer stagingBufferHandle;
        VmaAllocation stagingBufferAllocationHandle;

        void initStagingBuffer(uint32 size);
        void destroyStagingBuffer();
    };
}