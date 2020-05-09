#include "bzpch.h"

#include "Buffer.h"


namespace BZ {

    DataElement::DataElement(DataType dataType, DataElements dataElements, bool normalized) :
        dataType(dataType), dataElements(dataElements), normalized(normalized),
        sizeBytes(getDataTypeSizeBytes() * getElementCount()), offsetBytes(0) {
    }

    uint32 DataElement::getDataTypeSizeBytes() const {
        switch(dataType) {
            case DataType::Float32:
            case DataType::Int32:
            case DataType::Uint32:
                return 4;
            case DataType::Float16:
            case DataType::Int16:
            case DataType::Uint16:
                return 2;
            case DataType::Int8:
            case DataType::Uint8:
                return 1;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
                return 0;
        }
    }

    uint32 DataElement::getElementCount() const {
        switch(dataElements) {
            case DataElements::Scalar:
                return 1;
            case DataElements::Vec2:
                return 2;
            case DataElements::Vec3:
                return 3;
            case DataElements::Vec4:
            case DataElements::Mat2:
                return 4;
            case DataElements::Mat3:
                return 9;
            case DataElements::Mat4:
                return 16;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return 0;
        }
    }

    VkFormat DataElement::toVkFormat() const {
        switch(dataType) {
            case DataType::Float32: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return VK_FORMAT_R32_SFLOAT;
                    case DataElements::Vec2:
                        return VK_FORMAT_R32G32_SFLOAT;
                    case DataElements::Vec3:
                        return VK_FORMAT_R32G32B32_SFLOAT;
                    case DataElements::Vec4:
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Float16: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return VK_FORMAT_R16_SFLOAT;
                    case DataElements::Vec2:
                        return VK_FORMAT_R16G16_SFLOAT;
                    case DataElements::Vec3:
                        return VK_FORMAT_R16G16B16_SFLOAT;
                    case DataElements::Vec4:
                        return VK_FORMAT_R16G16B16A16_SFLOAT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Int32: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return VK_FORMAT_R32_SINT;
                    case DataElements::Vec2:
                        return VK_FORMAT_R32G32_SINT;
                    case DataElements::Vec3:
                        return VK_FORMAT_R32G32B32_SINT;
                    case DataElements::Vec4:
                        return VK_FORMAT_R32G32B32A32_SINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Int16: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return normalized ? VK_FORMAT_R16_SNORM : VK_FORMAT_R16_SINT;
                    case DataElements::Vec2:
                        return normalized ? VK_FORMAT_R16G16_SNORM : VK_FORMAT_R16G16_SINT;
                    case DataElements::Vec3:
                        return normalized ? VK_FORMAT_R16G16B16_SNORM : VK_FORMAT_R16G16B16_SINT;
                    case DataElements::Vec4:
                        return normalized ? VK_FORMAT_R16G16B16A16_SNORM : VK_FORMAT_R16G16B16A16_SINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Int8: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return normalized ? VK_FORMAT_R8_SNORM : VK_FORMAT_R8_SINT;
                    case DataElements::Vec2:
                        return normalized ? VK_FORMAT_R8G8_SNORM : VK_FORMAT_R8G8_SINT;
                    case DataElements::Vec3:
                        return normalized ? VK_FORMAT_R8G8B8_SNORM : VK_FORMAT_R8G8B8_SINT;
                    case DataElements::Vec4:
                        return normalized ? VK_FORMAT_R8G8B8A8_SNORM : VK_FORMAT_R8G8B8A8_SINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Uint32: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return VK_FORMAT_R32_UINT;
                    case DataElements::Vec2:
                        return VK_FORMAT_R32G32_UINT;
                    case DataElements::Vec3:
                        return VK_FORMAT_R32G32B32_UINT;
                    case DataElements::Vec4:
                        return VK_FORMAT_R32G32B32A32_UINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Uint16: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return normalized ? VK_FORMAT_R16_UNORM : VK_FORMAT_R16_UINT;
                    case DataElements::Vec2:
                        return normalized ? VK_FORMAT_R16G16_UNORM : VK_FORMAT_R16G16_UINT;
                    case DataElements::Vec3:
                        return normalized ? VK_FORMAT_R16G16B16_UNORM : VK_FORMAT_R16G16B16_UINT;
                    case DataElements::Vec4:
                        return normalized ? VK_FORMAT_R16G16B16A16_UNORM : VK_FORMAT_R16G16B16A16_UINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            case DataType::Uint8: {
                switch(dataElements) {
                    case DataElements::Scalar:
                        return normalized ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_UINT;
                    case DataElements::Vec2:
                        return normalized ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_UINT;
                    case DataElements::Vec3:
                        return normalized ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_UINT;
                    case DataElements::Vec4:
                        return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT;
                    case DataElements::Mat2:
                    case DataElements::Mat3:
                    case DataElements::Mat4:
                        BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                        return VK_FORMAT_UNDEFINED;
                    default:
                        BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                        return VK_FORMAT_UNDEFINED;
                }
            }
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
                return VK_FORMAT_UNDEFINED;
        }
    }


    /*-------------------------------------------------------------------------------------------*/
    DataLayout::DataLayout(const std::initializer_list<DataElement>& elements, VkVertexInputRate vertexInputRate) :
        elements(elements), sizeBytes(0), vertexInputRate(vertexInputRate) {
        BZ_ASSERT_CORE(elements.size() > 0, "DataLayout created without DataElements!");
        calculateOffsetsAndStride();
    }

    void DataLayout::calculateOffsetsAndStride() {
        uint32 offsetBytes = 0;
        for(auto& element : elements) {
            element.offsetBytes = offsetBytes;
            offsetBytes += element.sizeBytes;
            sizeBytes += element.sizeBytes;
        }
    }


    /*-------------------------------------------------------------------------------------------*/
    BufferPtr::operator void*() const {
        BZ_ASSERT_CORE(buffer && basePtr, "BufferPtr is not initialized!");
        return basePtr + buffer->getCurrentBaseOfReplicaOffset();
    }

    BufferPtr::operator byte*() const {
        BZ_ASSERT_CORE(buffer && basePtr, "BufferPtr is not initialized!");
        return basePtr + buffer->getCurrentBaseOfReplicaOffset();
    }

    BufferPtr operator+(const BufferPtr &lhs, uint32 offset) {
        return BufferPtr(*lhs.buffer, lhs.basePtr + offset);
    }

    BufferPtr operator-(const BufferPtr &lhs, uint32 offset) {
        return BufferPtr(*lhs.buffer, lhs.basePtr - offset);
    }


    /*-------------------------------------------------------------------------------------------*/
    Ref<Buffer> Buffer::create(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType) {
        return MakeRef<Buffer>(usageFlags, size, memoryType, nullptr);
    }

    Ref<Buffer> Buffer::create(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType, const DataLayout &layout) {
        return MakeRef<Buffer>(usageFlags, size, memoryType, &layout);
    }

    Buffer::Buffer(VkBufferUsageFlags usageFlags, uint32 size, MemoryType memoryType, const DataLayout *layout) :
        usageFlags(usageFlags), size(size), memoryType(memoryType) {

        if(isDynamic())
            realSize = size * MAX_FRAMES_IN_FLIGHT;
        else
            realSize = size;

        if(layout) {
            BZ_ASSERT_CORE(isVertex() || isIndex(), "Buffer with layout must be Vertex or Index!");

            if(isIndex())
                BZ_ASSERT_CORE(layout->begin()->getDataType() == DataType::Uint16 || layout->begin()->getDataType() == DataType::Uint32,
                    "Index Buffer Datatype must be Uint32 or Uint16!");

            this->layout = *layout;
        }
        else {
            BZ_ASSERT_CORE(!isVertex() || !isIndex(), "Buffer without layout cannot be Vertex or Index!");
        }

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = realSize;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = toRequiredVkBufferUsageFlags();
        allocInfo.preferredFlags = toPreferredVkBufferUsageFlags();
        BZ_ASSERT_VK(vmaCreateBuffer(getGraphicsContext().getMemoryAllocator(), &bufferInfo, &allocInfo, &handle.bufferHandle, &handle.allocationHandle, nullptr));
    }

    Buffer::~Buffer() {
        if(isMapped)
            unmap();

        vmaDestroyBuffer(getGraphicsContext().getMemoryAllocator(), handle.bufferHandle, handle.allocationHandle);
    }

    void Buffer::setData(const void *data, uint32 dataSize, uint32 offset) {
        BZ_ASSERT_CORE(memoryType != MemoryType::GpuToCpu, "Can't setData() on a MemoryType::GpuToCpu buffer!");
        BZ_ASSERT_CORE(data, "Data is null!");
        BZ_ASSERT_CORE(offset >= 0 && offset + dataSize <= this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(dataSize > 0, "Size is not valid!");
        BZ_ASSERT_CORE(!isMapped, "Buffer is being mapped!");

        if(memoryType == MemoryType::GpuOnly) {
            Buffer stagingBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, dataSize, MemoryType::CpuToGpu, nullptr);
            stagingBuffer.setData(data, dataSize, offset);

            //Transfer from staging buffer to device local buffer.
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = getGraphicsContext().getCurrentFrameCommandPool(QueueProperty::Transfer, true).getHandle();
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(getVkDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = offset;
            copyRegion.size = dataSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer.handle.bufferHandle, handle.bufferHandle, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            auto transferQueueHandle = getGraphicsContext().getDevice().getQueueContainerExclusive().transfer().getHandle();
            vkQueueSubmit(transferQueueHandle, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(transferQueueHandle);

            vkFreeCommandBuffers(getVkDevice(), allocInfo.commandPool, 1, &commandBuffer);
        }
        else {
            void *ptr;
            BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.allocationHandle, &ptr));
            memcpy(static_cast<byte*>(ptr) + offset, data, dataSize);
            vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.allocationHandle);
        }
    }

    BufferPtr Buffer::map(uint32 offset) {
        BZ_ASSERT_CORE(offset >= 0 && offset < this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(!isMapped, "Buffer already mapped!");
        BZ_ASSERT_CORE(isDynamic(), "Can't map buffer marked with non-dynamic.");

        isMapped = true;

        void *ptr;
        BZ_ASSERT_VK(vmaMapMemory(getGraphicsContext().getMemoryAllocator(), handle.allocationHandle, &ptr));
        return BufferPtr(*this, static_cast<byte*>(ptr) + offset);
    }

    void Buffer::unmap() {
        BZ_ASSERT_CORE(isMapped, "Buffer not mapped!");

        isMapped = false;
        vmaUnmapMemory(getGraphicsContext().getMemoryAllocator(), handle.allocationHandle);
    }

    uint32 Buffer::getCurrentBaseOfReplicaOffset() const {
        return isDynamic() ? getGraphicsContext().getCurrentFrameIndex() * this->size : 0;
    }

    VkMemoryPropertyFlags Buffer::toRequiredVkBufferUsageFlags() const {
        switch(memoryType) {
            case MemoryType::GpuOnly:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case MemoryType::CpuToGpu:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            case MemoryType::GpuToCpu:
                return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown MemoryType!");
                return 0;
        }
    }

    VkMemoryPropertyFlags Buffer::toPreferredVkBufferUsageFlags() const {
        switch(memoryType) {
            case MemoryType::GpuOnly:
            case MemoryType::CpuToGpu:
                return 0;
            case MemoryType::GpuToCpu:
                return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown MemoryType!");
                return 0;
        }
    }
}