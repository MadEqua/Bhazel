#include "bzpch.h"

#include "Buffer.h"
#include "Graphics/Graphics.h"
#include "Constants.h"

//#include "Platform/OpenGL/OpenGLBuffer.h"
//#include "Platform/D3D11/D3D11Buffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


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

    Ref<Buffer> Buffer::createVertexBuffer(const void *data, uint32 size, const DataLayout &layout, bool dynamic) {
        return create(BufferType::Vertex, size, data, layout, dynamic); 
    }

    Ref<Buffer> Buffer::createIndexBuffer(const void *data, uint32 size, bool dynamic) {
        return create(BufferType::Index, size, data,  dynamic);
    }

    Ref<Buffer> Buffer::createConstantBuffer(const void *data, uint32 size, bool dynamic) {
        return create(BufferType::Constant, size, data, dynamic);
    }

    Ref<Buffer> Buffer::createConstantBuffer(uint32 size, bool dynamic) {
        return create(BufferType::Constant, size, nullptr, dynamic);
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data, bool dynamic) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, data, nullptr, dynamic);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data, const DataLayout &layout, bool dynamic) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, data, &layout, dynamic);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Buffer::Buffer(BufferType type, uint32 size, const DataLayout *layout, bool dynamic) :
        type(type), dynamic(dynamic), size(size) {

        if(dynamic)
            realSize = size * MAX_FRAMES_IN_FLIGHT;
        else
            realSize = size;

        if(layout)
            this->layout = *layout;
    }

    void Buffer::initBufferData(const void *data) {
        if(data) {
            if(dynamic)
                for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                    uint32 baseOfReplica = i * size;
                    internalSetData(data, baseOfReplica, size);
                }
            else
                internalSetData(data, 0, size);
        }
    }

    void Buffer::setData(const void *data, uint32 offset, uint32 size) {
        BZ_ASSERT_CORE(data, "Data is null!");
        BZ_ASSERT_CORE(offset >= 0 && offset < this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(size > 0, "Size is not valid!");

        uint32 baseOfReplica = dynamic ? Application::getInstance().getGraphicsContext().getCurrentFrameIndex() * this->size : 0;
        internalSetData(data, baseOfReplica + offset, size);
    }

    void* Buffer::map(uint32 offset, uint32 size) {
        BZ_ASSERT_CORE(offset >= 0 && offset < this->size, "Offset is not valid!");
        BZ_ASSERT_CORE(size > 0, "Size is not valid!");
        BZ_ASSERT_CORE(!isMapped, "Buffer already mapped!");

        isMapped = true;
        uint32 baseOfReplica = dynamic ? Application::getInstance().getGraphicsContext().getCurrentFrameIndex() * this->size : 0;
        return internalMap(baseOfReplica + offset, size);
    }

    void Buffer::unmap() {
        BZ_ASSERT_CORE(isMapped, "Buffer not mapped!");

        isMapped = false;
        internalUnmap();
    }
}