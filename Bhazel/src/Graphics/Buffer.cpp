#include "bzpch.h"

#include "Buffer.h"
#include "Graphics/Graphics.h"

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

    Ref<Buffer> Buffer::createVertexBuffer(const void *data, uint32 size, const DataLayout &layout, bool dynamic) {
        return create(BufferType::Vertex, size, data, layout, dynamic); 
    }

    Ref<Buffer> Buffer::createIndexBuffer(const void *data, uint32 size, bool dynamic) {
        return create(BufferType::Index, size, data,  dynamic);
    }

    Ref<Buffer> Buffer::createConstantBuffer(uint32 size, bool dynamic) {
        return create(BufferType::Constant, size, nullptr, dynamic);
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data, bool dynamic) {
        switch(Graphics::api) {
        /*case Graphics::API::OpenGL:
            return MakeRef<OpenGLBuffer>(type, size);
        case Graphics::API::D3D11:
            return MakeRef<D3D11Buffer>(type, size);*/
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, data, dynamic);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data, const DataLayout&layout, bool dynamic) {
        switch(Graphics::api) {
        /*case Graphics::API::OpenGL:
            return MakeRef<OpenGLBuffer>(type, size, data, layout);
        case Graphics::API::D3D11:
            return MakeRef<D3D11Buffer>(type, size, data, layout);*/
        case Graphics::API::Vulkan:
            return MakeRef<VulkanBuffer>(type, size, data, layout, dynamic);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Buffer::Buffer(BufferType type, uint32 size, bool dynamic) :
        type(type), 
        size(size),
        dynamic(dynamic) {
    }

    Buffer::Buffer(BufferType type, uint32 size, const DataLayout &layout, bool dynamic) : 
        type(type),
        layout(layout), dynamic(dynamic) {
    }
}