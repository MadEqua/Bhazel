#include "bzpch.h"

#include "Buffer.h"

#ifdef BZ_PLATFORM_VULKAN
#include "Platform/Vulkan/VulkanBuffer.h"
#endif


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


    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::createVertexBuffer(const void *data, uint32 size, const DataLayout &layout) {
        return create(BufferType::Vertex, size, data, layout); 
    }

    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::createIndexBuffer(const void *data, uint32 size) {
        return create(BufferType::Index, size, data);
    }

    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::createConstantBuffer(uint32 size) {
        return create(BufferType::Constant, size);
    }

    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::create(BufferType type, uint32 size) {
#ifdef BZ_PLATFORM_VULKAN
        return MakeRef<VulkanBuffer>(type, size, nullptr, DataLayout());
#endif
    }

    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::create(BufferType type, uint32 size, const void *data) {
#ifdef BZ_PLATFORM_VULKAN
        return MakeRef<VulkanBuffer>(type, size, data, DataLayout());
#endif
    }

    template <typename NativeHandle>
    Ref<Buffer<NativeHandle>> Buffer<NativeHandle>::create(BufferType type, uint32 size, const void *data, const DataLayout &layout) {
#ifdef BZ_PLATFORM_VULKAN
        return MakeRef<VulkanBuffer>(type, size, data, layout);
#endif
    }

    template<typename NativeHandle>
    Buffer<NativeHandle>::Buffer(BufferType type, uint32 size, const void *data, const DataLayout &layout) :
        type(type), size(size), data(data), layout(layout) {
    }

#ifdef BZ_PLATFORM_VULKAN
    template class Buffer<VulkanBufferHandlesContainer>;
#endif
}