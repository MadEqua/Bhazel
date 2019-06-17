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

        virtual void setLayout(const BufferLayout &layout) override;
        virtual const BufferLayout& getLayout() const override { return layout; };

    private:
        GLuint rendererId;
        BufferLayout layout;
    };


    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(unsigned int *indices, unsigned int count);
        virtual ~OpenGLIndexBuffer() override;

        virtual unsigned int getCount() const override {
            return count;
        }

        virtual void bind() const override;
        virtual void unbind() const override;

    private:
        GLuint rendererId;
        unsigned int count;
    };
}