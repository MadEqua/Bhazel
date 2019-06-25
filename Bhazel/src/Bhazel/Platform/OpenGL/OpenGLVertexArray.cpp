#include "bzpch.h"
#include "OpenGLVertexArray.h"

namespace BZ {

    OpenGLVertexArray::OpenGLVertexArray() {
        glGenVertexArrays(1, &rendererId);
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &rendererId);
    }

    void OpenGLVertexArray::bind() const {
        glBindVertexArray(rendererId);
    }

    void OpenGLVertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer> &buffer) {
        BZ_CORE_ASSERT(buffer->getLayout().getElements().size(), "VertexBuffer has no layout.");

        glBindVertexArray(rendererId);
        buffer->bind();

        int index = 0;
        for(const auto &element : buffer->getLayout()) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, element.getElementCount(),
                                  shaderDataTypeToGL(element.dataType),
                                  element.normalized ? GL_TRUE : GL_FALSE,
                                  buffer->getLayout().getStride(), reinterpret_cast<const void*>(element.offset));
            index++;
        }

        vertexBuffers.emplace_back(buffer);
    }

    void OpenGLVertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer) {
        glBindVertexArray(rendererId);
        buffer->bind();

        indexBuffer = buffer;
    }

    GLenum OpenGLVertexArray::shaderDataTypeToGL(ShaderDataType dataType) {
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
            BZ_CORE_ASSERT_ALWAYS("Unknown ShaderDataType.");
            return 0;
        }
    }
}