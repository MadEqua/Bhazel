#include "bzpch.h"

#include "VulkanBuffer.h"
#include "Platform/Vulkan/VulkanConversions.h"


namespace BZ {

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size) :
        VulkanBuffer(type, size, nullptr, DataLayout()) {
    }

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size, const void *data) :
        VulkanBuffer(type, size, data, DataLayout()) {
    }

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size, const void* data, const DataLayout& layout) :
        Buffer(type, size, layout) {

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = bufferTypeToVK(type);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO

        BZ_ASSERT_VK(vkCreateBuffer(getDevice(), &bufferInfo, nullptr, &nativeHandle));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(getDevice(), nativeHandle, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        //TODO: pick correct flags for usage
        allocInfo.memoryTypeIndex = getGraphicsContext().findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        BZ_ASSERT_VK(vkAllocateMemory(getDevice(), &allocInfo, nullptr, &memoryHandle));
        BZ_ASSERT_VK(vkBindBufferMemory(getDevice(), nativeHandle, memoryHandle, 0));

        if(data)
            setData(data, size);
    }

    VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(getDevice(), nativeHandle, nullptr);
        vkFreeMemory(getDevice(), memoryHandle, nullptr);
    }

    void VulkanBuffer::setData(const void *data, uint32 size) {
        BZ_ASSERT_CORE(data, "Data is null!")
        BZ_ASSERT_CORE(data > 0, "Data size is not valid!")

        void *ptr;
        BZ_ASSERT_VK(vkMapMemory(getDevice(), memoryHandle, 0, size, 0, &ptr));
        memcpy(ptr, data, size);
        vkUnmapMemory(getDevice(), memoryHandle);
    }
}