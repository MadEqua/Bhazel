#include "bzpch.h"

#include "VulkanBuffer.h"


namespace BZ {

    static uint32 bufferTypeToVK(BufferType bufferType);


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

        BZ_ASSERT_VK(vkCreateBuffer(getGraphicsContext().getDevice(), &bufferInfo, nullptr, &nativeHandle));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(getGraphicsContext().getDevice(), nativeHandle, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        //TODO: pick correct flags for usage
        allocInfo.memoryTypeIndex = getGraphicsContext().findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        BZ_ASSERT_VK(vkAllocateMemory(getGraphicsContext().getDevice(), &allocInfo, nullptr, &memoryHandle));
        BZ_ASSERT_VK(vkBindBufferMemory(getGraphicsContext().getDevice(), nativeHandle, memoryHandle, 0));

        setData(data, size);
    }

    VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(getGraphicsContext().getDevice(), nativeHandle, nullptr);
        vkFreeMemory(getGraphicsContext().getDevice(), memoryHandle, nullptr);
    }

    void VulkanBuffer::setData(const void *data, uint32 size) {
        void *ptr;
        BZ_ASSERT_VK(vkMapMemory(getGraphicsContext().getDevice(), memoryHandle, 0, size, 0, &ptr));
        memcpy(ptr, data, size);
        vkUnmapMemory(getGraphicsContext().getDevice(), memoryHandle);
    }

    static uint32 bufferTypeToVK(BufferType bufferType) {
        switch(bufferType) {
        case BufferType::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BZ::BufferType::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BZ::BufferType::Constant:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
            return 0;
        }
    }
}