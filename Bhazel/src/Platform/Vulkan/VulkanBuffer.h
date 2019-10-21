#pragma once

#include "Graphics/Buffer.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    class VulkanBuffer : public Buffer, public VulkanGpuObject<VkBuffer> {
    public:
        VulkanBuffer(BufferType type, uint32 size);
        VulkanBuffer(BufferType type, uint32 size, const void *data);
        VulkanBuffer(BufferType type, uint32 size, const void *data, const DataLayout&layout);
        virtual ~VulkanBuffer() override;

        virtual void setData(const void *data, uint32 size, uint32 offset = 0) override;

    private:
        VkDeviceMemory memoryHandle;
    };
}