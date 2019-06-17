#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include <glad/glad.h>

namespace BZ {
    
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float *vertices, uint32 size);
        virtual ~OpenGLVertexBuffer() override;

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        GLuint rendererId;
    };


    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32 *indices, uint32 count);
        virtual ~OpenGLIndexBuffer() override;

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        GLuint rendererId;
    };
}