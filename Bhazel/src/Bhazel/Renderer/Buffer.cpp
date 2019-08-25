#include "bzpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLBuffer.h"

namespace BZ {

    uint32 BufferElement::getElementCount() const {
        switch(dataType)
        {
        case ShaderDataType::Float:
        case ShaderDataType::Int:
        case ShaderDataType::Bool:
            return 1;
        case ShaderDataType::Vec2i:
        case ShaderDataType::Vec2:
            return 2;
        case ShaderDataType::Vec3i:
        case ShaderDataType::Vec3:
            return 3;
        case ShaderDataType::Vec4i:
        case ShaderDataType::Vec4:
            return 4;
        case ShaderDataType::Mat2:
            return 4;
        case ShaderDataType::Mat3:
            return 9;
        case ShaderDataType::Mat4:
            return 16;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown ShaderDataType.");
            return 0;
        }
    }


    void BufferLayout::calculateOffsetsAndStride() {
        uint32 offset = 0;
        for(auto &element : elements) {
            element.offset = offset;
            offset += element.sizeBytes;
            stride += element.sizeBytes;
        }
    }


    Ref<VertexBuffer> VertexBuffer::create(float *vertices, uint32 size) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLVertexBuffer>(vertices, size);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    Ref<IndexBuffer> IndexBuffer::create(uint32 *indices, uint32 count) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::API::OpenGL:
            return MakeRef<OpenGLIndexBuffer>(indices, count);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }
}