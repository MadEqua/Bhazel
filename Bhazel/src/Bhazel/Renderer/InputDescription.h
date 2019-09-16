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

        virtual void addVertexBuffer(const Ref<Buffer> &buffer, const Ref<Shader> &vertexShader) = 0;
        virtual void setIndexBuffer(const Ref<Buffer> &buffer) = 0;

        std::vector<Ref<Buffer>>& getVertexBuffers() { return vertexBuffers; }
        Ref<Buffer>& getIndexBuffer() { return indexBuffer; }
        bool hasIndexBuffer() { return static_cast<bool>(indexBuffer); }

        static Ref<InputDescription> create();

    protected:
        std::vector<Ref<Buffer>> vertexBuffers;
        Ref<Buffer> indexBuffer;
    };
}