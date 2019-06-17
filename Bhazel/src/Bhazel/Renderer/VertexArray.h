#pragma once

#include <memory>
#include "Buffer.h"

namespace BZ {

    class VertexArray
    {
    public:
        virtual ~VertexArray() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void addVertexBuffer(std::shared_ptr<VertexBuffer> buffer) = 0;
        virtual void setIndexBuffer(std::shared_ptr<IndexBuffer> buffer) = 0;

        virtual std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() = 0;
        virtual std::shared_ptr<IndexBuffer>& getIndexBuffer() = 0;

        static VertexArray* create();
    };
}