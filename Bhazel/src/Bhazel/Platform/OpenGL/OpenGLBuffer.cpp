#include "bzpch.h"

#include "OpenGLBuffer.h"

namespace BZ {

    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32 size, const BufferLayout &layout) :
        VertexBuffer(layout) {
        BZ_ASSERT_GL(glGenBuffers(1, &rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ARRAY_BUFFER, rendererId));
        BZ_ASSERT_GL(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        BZ_ASSERT_GL(glDeleteBuffers(1, &rendererId));
    }


    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32 *indices, uint32 count) : 
        IndexBuffer(count) {
        BZ_ASSERT_GL(glGenBuffers(1, &rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId));
        BZ_ASSERT_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * count, indices, GL_STATIC_DRAW));
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        BZ_ASSERT_GL(glDeleteBuffers(1, &rendererId));
    }


    OpenGLConstantBuffer::OpenGLConstantBuffer(uint32 size) :
        OpenGLConstantBuffer(nullptr, size) {
    }

    OpenGLConstantBuffer::OpenGLConstantBuffer(void *data, uint32 size) :
        ConstantBuffer(size) {
        BZ_ASSERT_GL(glGenBuffers(1, &rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_UNIFORM_BUFFER, rendererId));
        BZ_ASSERT_GL(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW));
    }

    OpenGLConstantBuffer::~OpenGLConstantBuffer() {
        BZ_ASSERT_GL(glDeleteBuffers(1, &rendererId));
    }

    void OpenGLConstantBuffer::bindToPipeline(uint32 unit) const {
        BZ_ASSERT_GL(glBindBufferBase(GL_UNIFORM_BUFFER, unit, rendererId));
    }

    void OpenGLConstantBuffer::unbindFromPipeline(uint32 unit) const {
        BZ_ASSERT_GL(glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0));
    }

    void OpenGLConstantBuffer::setData(const void *data, uint32 size) {
        BZ_ASSERT_GL(glBindBuffer(GL_UNIFORM_BUFFER, rendererId));
        BZ_ASSERT_GL(glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data));
    }
}