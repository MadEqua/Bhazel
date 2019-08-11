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

        virtual void addVertexBuffer(const Ref<VertexBuffer> &buffer) override;
        virtual void setIndexBuffer(const Ref<IndexBuffer> &buffer) override;

    private:
        GLuint rendererId;

        static GLenum shaderDataTypeToGL(ShaderDataType dataType);
    };
}