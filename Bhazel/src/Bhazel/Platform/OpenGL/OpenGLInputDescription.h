#pragma once

#include "Bhazel/Renderer/InputDescription.h"
#include "Bhazel/Renderer/Buffer.h"

#include "OpenGLIncludes.h"


namespace BZ {

    class OpenGLInputDescription : public InputDescription
    {
    public:
        OpenGLInputDescription();
        virtual ~OpenGLInputDescription() override;

        virtual void bind() const override;
        virtual void unbind() const override;

        virtual void addVertexBuffer(const Ref<VertexBuffer> &buffer, const Ref<Shader> &vertexShader) override;
        virtual void setIndexBuffer(const Ref<IndexBuffer> &buffer) override;

    private:
        GLuint rendererId;

        static GLenum shaderDataTypeToGL(ShaderDataType dataType);
    };
}