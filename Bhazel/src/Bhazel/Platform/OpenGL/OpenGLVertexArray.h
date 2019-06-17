#pragma once

#include "Bhazel/Renderer/VertexArray.h"
#include "Bhazel/Renderer/Buffer.h"

#include <memory>
#include <glad/glad.h>

namespace BZ {

    class OpenGLVertexArray : public VertexArray
    {
    public:
        OpenGLVertexArray();
        virtual ~OpenGLVertexArray() override;

        virtual void bind() const override;
        virtual void unbind() const override;

        virtual void addVertexBuffer(std::shared_ptr<VertexBuffer> buffer) override;
        virtual void setIndexBuffer(std::shared_ptr<IndexBuffer> buffer) override;

        virtual std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() override { return vertexBuffers; }
        virtual std::shared_ptr<IndexBuffer>& getIndexBuffer() override { return indexBuffer; }

    private:
        std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
        std::shared_ptr<IndexBuffer> indexBuffer;

        GLuint rendererId;

        static GLenum shaderDataTypeToGL(ShaderDataType dataType);
    };
}