#include "bzpch.h"

#include "Buffer.h"
#include "Graphics/Graphics.h"
#include "Constants.h"

//#include "Platform/OpenGL/OpenGLBuffer.h"
//#include "Platform/D3D11/D3D11Buffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"


namespace BZ {

    DataElement::DataElement(DataType dataType, DataElements dataElements, const char *name, bool normalized) :
        dataType(dataType), dataElements(dataElements), name(name), normalized(normalized),
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


    DataLayout::DataLayout(const std::initializer_list<DataElement>& elements, DataRate dataRate) :
        elements(elements), sizeBytes(0), dataRate(dataRate) {
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


    BufferPtr::operator void*() const {
        return basePtr + buffer->getCurrentBaseOfReplicaOffset();
    }

    BufferPtr::operator byte*() const {
        return basePtr + buffer->getCurrentBaseOfReplicaOffset();
    }

    BufferPtr operator+(const BufferPtr &lhs, uint32 offset) {
        return BufferPtr(*lhs.buffer, lhs.basePtr + offset);
    }

    BufferPtr operator-(const BufferPtr &lhs, uint32 offset) {
        return BufferPtr(*lhs.buffer, lhs.basePtr - offset);
    }


    Ref<Buffer> Buffer::create(BufferType type, uint32 size, MemoryType memoryType) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, memoryType, nullptr);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, MemoryType memoryType, const DataLayout &layout) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, memoryType, &layout);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Buffer::Buffer(BufferType type, uint32 size, MemoryType memoryType, const DataLayout *layout) :
        type(type), size(size), memoryType(memoryType) {

        if(isDynamic())
            realSize = size * MAX_FRAMES_IN_FLIGHT;
        else
            realSize = size;

        if (layout) {
            BZ_ASSERT_CORE(type == BufferType::Vertex || type == BufferType::Index, "Buffer with layout must be Vertex or Index!");

            if(type == BufferType::Index)
                BZ_ASSERT_CORE(layout->begin()->dataType == DataType::Uint16 || layout->begin()->dataType == DataType::Uint32, "Index Buffer Datatype must be Uint32 or Uint16!");

            this->layout = *layout;
        }
        else {
            BZ_ASSERT_CORE(type != BufferType::Vertex && type != BufferType::Index, "Buffer without layout cannot be Vertex or Index!");
        }
    }

    /*void Buffer::initBufferData(const void *data) {
        if(data) {
            if(isDynamic())
                for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                    uint32 baseOfReplica = i * size;
                    internalSetData(data, baseOfReplica, size);
                }
            else
                internalSetData(data, 0, size);
        }
    }*/

    void Buffer::setData(const void *data, uint32 dataSize, uint32 offset) {
        BZ_ASSERT_CORE(data, "Data is null!");
        BZ_ASSERT_CORE(offset >= 0 && offset + dataSize <= this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(dataSize > 0, "Size is not valid!");
        BZ_ASSERT_CORE(!isMapped, "Buffer is being mapped!");

        internalSetData(data, dataSize, getCurrentBaseOfReplicaOffset() + offset);
    }

    BufferPtr Buffer::map(uint32 offset) {
        BZ_ASSERT_CORE(offset >= 0 && offset < this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(!isMapped, "Buffer already mapped!");
        BZ_ASSERT_CORE(isDynamic(), "Can't map buffer marked with non-dynamic.");

        isMapped = true;
        return internalMap(offset);
    }

    void Buffer::unmap() {
        BZ_ASSERT_CORE(isMapped, "Buffer not mapped!");

        isMapped = false;
        internalUnmap();
    }

    uint32 Buffer::getCurrentBaseOfReplicaOffset() const {
        return isDynamic() ? Application::getInstance().getGraphicsContext().getCurrentFrameIndex() * this->size : 0;
    }
}