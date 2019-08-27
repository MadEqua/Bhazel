#pragma once

#include "Buffer.h"


namespace BZ {

    class InputDescription
    {
    public:
        virtual ~InputDescription() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void addVertexBuffer(const Ref<VertexBuffer> &buffer) = 0;
        virtual void setIndexBuffer(const Ref<IndexBuffer> &buffer) = 0;

        std::vector<Ref<VertexBuffer>>& getVertexBuffers() { return vertexBuffers; }
        Ref<IndexBuffer>& getIndexBuffer() { return indexBuffer; }

        static Ref<InputDescription> create();

    protected:
        std::vector<Ref<VertexBuffer>> vertexBuffers;
        Ref<IndexBuffer> indexBuffer;
    };
}