#include "bzpch.h"

#include "VulkanBuffer.h"
#include "Bhazel/Application.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"


namespace BZ {

    static uint32 bufferTypeToVK(BufferType bufferType);


    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size) :
        VulkanBuffer(type, size, nullptr, DataLayout()) {
    }

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size, const void *data) :
        VulkanBuffer(type, size, data, DataLayout()) {
    }

    VulkanBuffer::VulkanBuffer(BufferType type, uint32 size, const void* data, const DataLayout& layout) :
        Buffer(type, size, layout),
        context(static_cast<VulkanContext&>(Application::getInstance().getGraphicsContext())) {

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = bufferTypeToVK(type);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO

        BZ_ASSERT_VK(vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &bufferHandle));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context.getDevice(), bufferHandle, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        //TODO: pick correct flags for usage
        allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        BZ_ASSERT_VK(vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &memoryHandle));
        BZ_ASSERT_VK(vkBindBufferMemory(context.getDevice(), bufferHandle, memoryHandle, 0));
    }

    VulkanBuffer::~VulkanBuffer() {
        vkDestroyBuffer(context.getDevice(), bufferHandle, nullptr);
        vkFreeMemory(context.getDevice(), memoryHandle, nullptr);
    }

    void VulkanBuffer::setData(const void *data, uint32 size) {
        void *ptr;
        BZ_ASSERT_VK(vkMapMemory(context.getDevice(), memoryHandle, 0, size, 0, &ptr));
        memcpy(ptr, data, size);
        vkUnmapMemory(context.getDevice(), memoryHandle);
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