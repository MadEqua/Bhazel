#pragma once

#include "Graphics/Buffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    class VulkanBuffer : public Buffer, public VulkanGpuObject<VkBuffer> {
    public:
        VulkanBuffer(BufferType type, uint32 size, const void *data, const DataLayout *layout, bool dynamic);
        virtual ~VulkanBuffer() override;

        void internalSetData(const void *data, uint32 offset, uint32 size) override;
        void* internalMap(uint32 offset, uint32 size) override;
        void internalUnmap() override;

    private:
        VkDeviceMemory memoryHandle;
    };
}