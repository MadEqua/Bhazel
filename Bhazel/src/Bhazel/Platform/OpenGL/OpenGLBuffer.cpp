#include "bzpch.h"

#include "OpenGLBuffer.h"

namespace BZ {

    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32 size) {
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
}