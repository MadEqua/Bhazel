#include "bzpch.h"

#include "VulkanBuffer.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"

#include "Graphics/Graphics.h"


namespace BZ {

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size, MemoryType memoryType, const DataLayout *layout) :
        Buffer(type, size, memoryType, layout) {

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = realSize;
        bufferInfo.usage = bufferTypeToVK(type) | (memoryType == MemoryType::GpuOnly ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(memoryType);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(memoryType);
        BZ_ASSERT_VK(vmaCreateBuffer(getGraphicsContext().getMemoryAllocator(), &bufferInfo, &allocInfo, &nativeHandle, &allocationHandle, nullptr));
    }

    VulkanBuffer::~VulkanBuffer() {
        vmaDestroyBuffer(getGraphicsContext().getMemoryAllocator(), nativeHandle, allocationHandle);
    }

    void VulkanBuffer::internalSetData(const void *data, uint32 offset, uint32 size) {
        VmaAllocation allocationHandle;
        if(memoryType == MemoryType::GpuOnly) {
            initStagingBuffer(size);
            allocationHandle = stagingBufferAllocationHandle;
        }
        else
            allocationHandle = this->allocationHandle;

        void *ptr;
        BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), allocationHandle, &ptr));
        memcpy(ptr, data, size);
        vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), allocationHandle);

        //Transfer from staging buffer to device local buffer.
        if(memoryType == MemoryType::GpuOnly) {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = getGraphicsContext().getCurrentFrameCommandPool(QueueProperty::Transfer, true).getNativeHandle();
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(getDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, stagingBufferHandle, this->nativeHandle, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            auto transferQueueHandle = getGraphicsContext().getDevice().getQueueContainerExclusive().transfer.getNativeHandle();
            vkQueueSubmit(transferQueueHandle, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(transferQueueHandle);

            vkFreeCommandBuffers(getDevice(), allocInfo.commandPool, 1, &commandBuffer);

            destroyStagingBuffer();
        }
    }

    byte* VulkanBuffer::internalMap(uint32 offset, uint32 size) {
        void *ptr;
        BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), allocationHandle, &ptr));
        return static_cast<byte*>(ptr);
    }

    void VulkanBuffer::internalUnmap() {
        vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), allocationHandle);
    }

    void VulkanBuffer::initStagingBuffer(uint32 size) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryTypeToRequiredFlagsVk(MemoryType::CpuToGpu);
        allocInfo.preferredFlags = memoryTypeToPreferredFlagsVk(MemoryType::CpuToGpu);
        BZ_ASSERT_VK(vmaCreateBuffer(getGraphicsContext().getMemoryAllocator(), &bufferInfo, &allocInfo, &stagingBufferHandle, &stagingBufferAllocationHandle, nullptr));
    }

    void VulkanBuffer::destroyStagingBuffer() {
        vmaDestroyBuffer(getGraphicsContext().getMemoryAllocator(), stagingBufferHandle, stagingBufferAllocationHandle);
    }
}