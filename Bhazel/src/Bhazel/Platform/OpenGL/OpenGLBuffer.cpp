#include "bzpch.h"

#include "OpenGLBuffer.h"

namespace BZ {

    static uint32 bufferTypeToGL(BufferType bufferType);


    OpenGLBuffer::OpenGLBuffer(BufferType type, uint32 size) :
        OpenGLBuffer(type, size, nullptr, BufferLayout()) {
    }

    OpenGLBuffer::OpenGLBuffer(BufferType type, uint32 size, const void *data) :
        OpenGLBuffer(type, size, data, BufferLayout()) {
    }

    OpenGLBuffer::OpenGLBuffer(BufferType type, uint32 size, const void *data, const BufferLayout &layout) :
        Buffer(type, size, layout) {

        uint32 bufferType = bufferTypeToGL(type);

        BZ_ASSERT_GL(glGenBuffers(1, &rendererId));
        BZ_ASSERT_GL(glBindBuffer(bufferType, rendererId));
        BZ_ASSERT_GL(glBufferData(bufferType, size, data, GL_DYNAMIC_DRAW)); //TODO: use the correct flag
    }

    OpenGLBuffer::~OpenGLBuffer() {
        BZ_ASSERT_GL(glDeleteBuffers(1, &rendererId));
    }

    void OpenGLBuffer::bindToPipeline(uint32 unit) const {
        switch(type) {
        case BZ::BufferType::Vertex:
        case BZ::BufferType::Index:
            //On OpenGL these buffers don't bind to pipeline binding points, they bind to VAOs (InputDescriptions).
            BZ_ASSERT_ALWAYS_CORE("Trying to bind vertex or index buffer to the GL pipeline!")
            break;
        case BZ::BufferType::Constant:
            BZ_ASSERT_GL(glBindBufferBase(GL_UNIFORM_BUFFER, unit, rendererId));
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
        }
    }

    void OpenGLBuffer::unbindFromPipeline(uint32 unit) const {
        switch(type) {
        case BZ::BufferType::Vertex:
        case BZ::BufferType::Index:
            //On OpenGL these buffers don't bind to pipeline binding points, they bind to VAOs (InputDescriptions).
            break;
        case BZ::BufferType::Constant:
            BZ_ASSERT_GL(glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0));
            break;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
        }
    }

    void OpenGLBuffer::bindToPipelineAsGeneric(uint32 unit) const {
        BZ_ASSERT_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, rendererId));
    }

    void OpenGLBuffer::unbindFromPipelineAsGeneric(uint32 unit) const {
        BZ_ASSERT_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, 0));
    }

    void OpenGLBuffer::setData(const void *data, uint32 size) {
        uint32 bufferType = bufferTypeToGL(type);
        BZ_ASSERT_GL(glBindBuffer(bufferType, rendererId));
        BZ_ASSERT_GL(glBufferSubData(bufferType, 0, size, data));
    }

    static uint32 bufferTypeToGL(BufferType bufferType) {
        switch(bufferType) {
        case BufferType::Vertex:
            return GL_ARRAY_BUFFER;
        case BZ::BufferType::Index:
            return GL_ELEMENT_ARRAY_BUFFER;
        case BZ::BufferType::Constant:
            return GL_UNIFORM_BUFFER;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
            return 0;
        }
    }
}