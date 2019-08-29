#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "OpenGLIncludes.h"

namespace BZ {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float *vertices, uint32 size, const BufferLayout &layout);
        virtual ~OpenGLVertexBuffer() override;

    private:
        GLuint rendererId;
        friend class OpenGLInputDescription;
    };


    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32 *indices, uint32 count);
        virtual ~OpenGLIndexBuffer() override;

    private:
        GLuint rendererId;
        friend class OpenGLInputDescription;
    };


    class OpenGLConstantBuffer : public ConstantBuffer {
    public:
        explicit OpenGLConstantBuffer(uint32 size);
        OpenGLConstantBuffer(void *data, uint32 size);
        virtual ~OpenGLConstantBuffer() override;

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;
        virtual void setData(const void *data, uint32 size) override;

        GLuint getNativeHandle() { return rendererId; }

    private:
        GLuint rendererId;
    };
}