#include "bzpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "Bhazel/Platform/OpenGL/OpenGLBuffer.h"

namespace BZ {

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