#pragma once

namespace BZ {

    class Buffer {
    public:
        virtual ~Buffer() = default;

        //virtual void setData() = 0;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;
    };


    class VertexBuffer : public Buffer {
    public:
        static VertexBuffer* create(float *vertices, unsigned int size);
    };


    class IndexBuffer : public Buffer {
    public:
        virtual unsigned int getCount() const = 0;
        static IndexBuffer* create(unsigned int *indices, unsigned int count);
    };
}