#pragma once

#include "Bhazel/Renderer/Buffer.h"

#include "OpenGLIncludes.h"

namespace BZ {

    class OpenGLBuffer : public Buffer {
    public:
        OpenGLBuffer(BufferType type, uint32 size);
        OpenGLBuffer(BufferType type, uint32 size, const void *data);
        OpenGLBuffer(BufferType type, uint32 size, const void *data, const BufferLayout &layout);
        virtual ~OpenGLBuffer() override;

        virtual void setData(const void *data, uint32 size) override;

        virtual void bindToPipeline(uint32 unit = 0) const override;
        virtual void unbindFromPipeline(uint32 unit = 0) const override;

        virtual void bindToPipelineAsGeneric(uint32 unit = 0) const override;
        virtual void unbindFromPipelineAsGeneric(uint32 unit = 0) const override;

    private:
        GLuint rendererId;
        friend class OpenGLInputDescription;
    };
}