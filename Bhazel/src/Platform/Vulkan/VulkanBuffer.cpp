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
        bufferInfo.usage = bufferTypeToVK(type) | (memoryType == MemoryType::Static ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO

        BZ_ASSERT_VK(vkCreateBuffer(getDevice(), &bufferInfo, nullptr, &nativeHandle));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(getDevice(), nativeHandle, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        allocInfo.memoryTypeIndex = getGraphicsContext().findMemoryType(memRequirements.memoryTypeBits, memoryType);

        BZ_ASSERT_VK(vkAllocateMemory(getDevice(), &allocInfo, nullptr, &memoryHandle));
        BZ_ASSERT_VK(vkBindBufferMemory(getDevice(), nativeHandle, memoryHandle, 0));
    }

    VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(getDevice(), nativeHandle, nullptr);
        vkFreeMemory(getDevice(), memoryHandle, nullptr);
    }

    void VulkanBuffer::internalSetData(const void *data, uint32 offset, uint32 size) {
        VkDeviceMemory memoryHandle;
        if(memoryType == MemoryType::Static) {
            initStagingBuffer(size);
            memoryHandle = stagingBufferMemoryHandle;
        }
        else
            memoryHandle = this->memoryHandle;

        void *ptr;
        BZ_ASSERT_VK(vkMapMemory(getDevice(), memoryHandle, offset, size, 0, &ptr));
        memcpy(ptr, data, size);
        vkUnmapMemory(getDevice(), memoryHandle);

        if(memoryType == MemoryType::Static) {
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

    void* VulkanBuffer::internalMap(uint32 offset, uint32 size) {
        void *ptr;
        BZ_ASSERT_VK(vkMapMemory(getDevice(), memoryHandle, offset, size, 0, &ptr));
        return ptr;
    }

    void VulkanBuffer::internalUnmap() {
        vkUnmapMemory(getDevice(), memoryHandle);
    }

    void VulkanBuffer::initStagingBuffer(uint32 size) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        BZ_ASSERT_VK(vkCreateBuffer(getDevice(), &bufferInfo, nullptr, &stagingBufferHandle));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(getDevice(), nativeHandle, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        allocInfo.memoryTypeIndex = getGraphicsContext().findMemoryType(memRequirements.memoryTypeBits, MemoryType::Write);

        BZ_ASSERT_VK(vkAllocateMemory(getDevice(), &allocInfo, nullptr, &stagingBufferMemoryHandle));
        BZ_ASSERT_VK(vkBindBufferMemory(getDevice(), stagingBufferHandle, stagingBufferMemoryHandle, 0));
    }

    void VulkanBuffer::destroyStagingBuffer() {
        vkDestroyBuffer(getDevice(), stagingBufferHandle, nullptr);
        vkFreeMemory(getDevice(), stagingBufferMemoryHandle, nullptr);
    }
}