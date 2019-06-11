#include "bzpch.h"

#include "OpenGLBuffer.h"

namespace BZ {

    OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, unsigned int size) {
        glCreateBuffers(1, &rendererId);
        bind();
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

    void OpenGLVertexBuffer::setLayout(const BufferLayout &layout) {
        //TODO: move from here to Vertex Array Object
        int index = 0;
        for(const auto &element : layout) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, element.getElementCount(),
                                  shaderDataTypeToGL(element.dataType), 
                                  element.normalized ?  GL_TRUE : GL_FALSE,
                                  layout.getStride(), reinterpret_cast<const void*>(element.offset));
            index++;
        }
        this->layout = layout;
    }

    GLenum OpenGLVertexBuffer::shaderDataTypeToGL(ShaderDataType dataType) {
        switch(dataType)
        {
        case ShaderDataType::Float:
        case ShaderDataType::Vec2:
        case ShaderDataType::Vec3:
        case ShaderDataType::Vec4:
        case ShaderDataType::Mat2:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            return GL_FLOAT;
        case ShaderDataType::Int:
        case ShaderDataType::Vec2i:
        case ShaderDataType::Vec3i:
        case ShaderDataType::Vec4i:
            return GL_INT;
        case ShaderDataType::Bool:
            return GL_BOOL;
        default:
            BZ_CORE_ASSERT(false, "Unknown ShaderDataType.");
            return 0;
        }
    }



    OpenGLIndexBuffer::OpenGLIndexBuffer(unsigned int *indices, unsigned int count) : 
        count(count) {
        glCreateBuffers(1, &rendererId);
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * count, indices, GL_STATIC_DRAW);
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