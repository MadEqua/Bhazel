#include "bzpch.h"

#include "OpenGLBuffer.h"

namespace BZ {

    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32 size, const BufferLayout &layout) :
        VertexBuffer(layout) {
        glGenBuffers(1, &rendererId);
        glBindBuffer(GL_ARRAY_BUFFER, rendererId);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        glDeleteBuffers(1, &rendererId);
    }

    void OpenGLVertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, rendererId);
    }

    void OpenGLVertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32 *indices, uint32 count) : 
        IndexBuffer(count) {
        glGenBuffers(1, &rendererId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * count, indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        glDeleteBuffers(1, &rendererId);
    }

    void OpenGLIndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId);
    }

    void OpenGLIndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }


    OpenGLConstantBuffer::OpenGLConstantBuffer(void *data, uint32 size) :
        ConstantBuffer(size) {
        glGenBuffers(1, &rendererId);
        glBindBuffer(GL_UNIFORM_BUFFER, rendererId);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_READ);
    }

    OpenGLConstantBuffer::~OpenGLConstantBuffer() {
        glDeleteBuffers(1, &rendererId);
    }

    void OpenGLConstantBuffer::bind() const {
        //glBindBuffer(GL_UNIFORM_BUFFER, rendererId);
    }

    void OpenGLConstantBuffer::unbind() const {
        //glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void OpenGLConstantBuffer::setData(void *data, uint32 size) {
    }
}