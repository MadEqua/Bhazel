#include "bzpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLBuffer.h"

namespace BZ {

    void BufferLayout::calculateOffsetsAndStride() {
        unsigned int offset = 0;
        for(auto &element : elements) {
            element.offset = offset;
            offset += element.sizeBytes;
            stride += element.sizeBytes;
        }
    }

    unsigned int BufferElement::getElementCount() const {
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
            BZ_CORE_ASSERT(false, "Unknown ShaderDataType.");
            return 0;
        }
    }


    VertexBuffer* VertexBuffer::create(float *vertices, unsigned int size) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::OpenGL:
            return new OpenGLVertexBuffer(vertices, size);
        case RendererAPI::None:
        default:
            BZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
            return nullptr;
        }
    }

    IndexBuffer* IndexBuffer::create(unsigned int *indices, unsigned int count) {
        switch(Renderer::getAPI())
        {
        case RendererAPI::OpenGL:
            return new OpenGLIndexBuffer(indices, count);
        case RendererAPI::None:
        default:
            BZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
            return nullptr;
        }
    }
}