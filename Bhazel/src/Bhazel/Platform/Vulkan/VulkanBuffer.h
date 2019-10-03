#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    class VulkanBuffer : public Buffer, public VulkanGpuObject<VkBuffer> {
    public:
        VulkanBuffer(BufferType type, uint32 size);
        VulkanBuffer(BufferType type, uint32 size, const void *data);
        VulkanBuffer(BufferType type, uint32 size, const void *data, const DataLayout&layout);
        virtual ~VulkanBuffer() override;

        virtual void setData(const void *data, uint32 size) override;

    private:
        VkDeviceMemory memoryHandle;
    };
}