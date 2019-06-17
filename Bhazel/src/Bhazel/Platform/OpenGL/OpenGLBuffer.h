#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include <glad/glad.h>

namespace BZ {
    
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float *vertices, unsigned int size);
        virtual ~OpenGLVertexBuffer() override;

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        GLuint rendererId;
    };


    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(unsigned int *indices, unsigned int count);
        virtual ~OpenGLIndexBuffer() override;

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        GLuint rendererId;
    };
}