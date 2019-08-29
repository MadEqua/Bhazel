#pragma once

#include "Buffer.h"
#include "Shader.h"


namespace BZ {

    class InputDescription
    {
    public:
        virtual ~InputDescription() = default;

        virtual void bindToPipeline() const = 0;
        virtual void unbindFromPipeline() const = 0;

        virtual void addVertexBuffer(const Ref<VertexBuffer> &buffer, const Ref<Shader> &vertexShader) = 0;
        virtual void setIndexBuffer(const Ref<IndexBuffer> &buffer) = 0;

        std::vector<Ref<VertexBuffer>>& getVertexBuffers() { return vertexBuffers; }
        Ref<IndexBuffer>& getIndexBuffer() { return indexBuffer; }

        static Ref<InputDescription> create();

    protected:
        std::vector<Ref<VertexBuffer>> vertexBuffers;
        Ref<IndexBuffer> indexBuffer;
    };
}