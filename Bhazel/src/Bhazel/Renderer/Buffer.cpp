#include "bzpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLBuffer.h"
#include "Bhazel/Platform/D3D11/D3D11Buffer.h"

namespace BZ {

    static uint32 shaderDataTypeSize(DataType type) {
        switch(type) {
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

    BufferElement::BufferElement(DataType dataType, DataElements dataElements,  const std::string &name, bool normalized, uint32 perInstanceStep) :
        dataType(dataType), dataElements(dataElements), name(name), normalized(normalized), perInstanceStep(perInstanceStep),
        sizeBytes(shaderDataTypeSize(dataType) * getElementCount()), offset(0) {
    }

    uint32 BufferElement::getElementCount() const {
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