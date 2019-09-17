#include "bzpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLBuffer.h"
#include "Bhazel/Platform/D3D11/D3D11Buffer.h"

namespace BZ {

    static uint32 shaderDataTypeSize(DataType type) {
        switch(type)
        {
        case DataType::Float:
            return sizeof(float);
        case DataType::Int:
            return sizeof(int);
        case DataType::Int16:
            return sizeof(int16);
        case DataType::Int8:
            return sizeof(int8);
        case DataType::Uint:
            return sizeof(uint32);
        case DataType::Uint16:
            return sizeof(uint16);
        case DataType::Uint8:
            return sizeof(uint8);
        case DataType::Bool:
            return sizeof(bool);
        case DataType::Vec2:
            return sizeof(float) * 2;
        case DataType::Vec3:
            return sizeof(float) * 3;
        case DataType::Vec4:
            return sizeof(float) * 4;
        case DataType::Vec2i:
            return sizeof(int) * 2;
        case DataType::Vec3i:
            return sizeof(int) * 3;
        case DataType::Vec4i:
            return sizeof(int) * 4;
        case DataType::Vec2ui:
            return sizeof(uint32) * 2;
        case DataType::Vec3ui:
            return sizeof(uint32) * 3;
        case DataType::Vec4ui:
            return sizeof(uint32) * 4;
        case DataType::Mat2:
            return sizeof(float) * 4;
        case DataType::Mat3:
            return sizeof(float) * 9;
        case DataType::Mat4:
            return sizeof(float) * 16;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return 0;
        }
    }

    BufferElement::BufferElement(DataType dataType, const std::string &name, bool normalized) :
        dataType(dataType), name(name), sizeBytes(shaderDataTypeSize(dataType)), offset(0), normalized(normalized) {
    }

    uint32 BufferElement::getElementCount() const {
        switch(dataType)
        {
        case DataType::Float:
        case DataType::Int:
        case DataType::Int16:
        case DataType::Int8:
        case DataType::Uint:
        case DataType::Uint16:
        case DataType::Uint8:
        case DataType::Bool:
            return 1;
        case DataType::Vec2i:
        case DataType::Vec2ui:
        case DataType::Vec2:
            return 2;
        case DataType::Vec3i:
        case DataType::Vec3ui:
        case DataType::Vec3:
            return 3;
        case DataType::Vec4i:
        case DataType::Vec4ui:
        case DataType::Vec4:
            return 4;
        case DataType::Mat2:
            return 4;
        case DataType::Mat3:
            return 9;
        case DataType::Mat4:
            return 16;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return 0;
        }
    }


    BufferLayout::BufferLayout(const std::initializer_list<BufferElement> &elements) :
        elements(elements), stride(0) {
        calculateOffsetsAndStride();
    }

    void BufferLayout::calculateOffsetsAndStride() {
        uint32 offset = 0;
        for(auto &element : elements) {
            element.offset = offset;
            offset += element.sizeBytes;
            stride += element.sizeBytes;
        }
    }


    Ref<Buffer> Buffer::createVertexBuffer(const void *data, uint32 size, const BufferLayout &layout) { 
        return create(BufferType::Vertex, size, data, layout); 
    }

    Ref<Buffer> Buffer::createIndexBuffer(const void *data, uint32 size) { 
        return create(BufferType::Index, size, data);
    }

    Ref<Buffer> Buffer::createConstantBuffer(uint32 size) { 
        return create(BufferType::Constant, size);
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size) {
        switch(Renderer::api) {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLBuffer>(type, size);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Buffer>(type, size);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data) {
        switch(Renderer::api) {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLBuffer>(type, size, data);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Buffer>(type, size, data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<Buffer> Buffer::create(BufferType type, uint32 size, const void *data, const BufferLayout &layout) {
        switch(Renderer::api) {
        case Renderer::API::OpenGL:
            return MakeRef<OpenGLBuffer>(type, size, data, layout);
        case Renderer::API::D3D11:
            return MakeRef<D3D11Buffer>(type, size, data, layout);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}