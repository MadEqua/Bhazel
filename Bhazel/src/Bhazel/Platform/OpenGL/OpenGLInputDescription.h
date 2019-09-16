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

        virtual void bindToPipeline() const override;
        virtual void unbindFromPipeline() const override;

        virtual void addVertexBuffer(const Ref<Buffer> &buffer, const Ref<Shader> &vertexShader) override;
        virtual void setIndexBuffer(const Ref<Buffer> &buffer) override;

    private:
        GLuint rendererId;

        static GLenum shaderDataTypeToGL(DataType dataType);
    };
}