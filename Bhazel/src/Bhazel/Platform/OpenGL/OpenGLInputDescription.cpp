#include "bzpch.h"
#include "OpenGLInputDescription.h"

namespace BZ {

    OpenGLInputDescription::OpenGLInputDescription() {
        glGenVertexArrays(1, &rendererId);
    }

    OpenGLInputDescription::~OpenGLInputDescription() {
        glDeleteVertexArrays(1, &rendererId);
    }

    void OpenGLInputDescription::bind() const {
        glBindVertexArray(rendererId);
    }

    void OpenGLInputDescription::unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLInputDescription::addVertexBuffer(const Ref<VertexBuffer> &buffer) {
        BZ_ASSERT_CORE(buffer->getLayout().getElements().size(), "VertexBuffer has no layout.");

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

    void OpenGLInputDescription::setIndexBuffer(const Ref<IndexBuffer> &buffer) {
        glBindVertexArray(rendererId);
        buffer->bind();

        indexBuffer = buffer;
    }

    GLenum OpenGLInputDescription::shaderDataTypeToGL(ShaderDataType dataType) {
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
            BZ_ASSERT_ALWAYS_CORE("Unknown ShaderDataType.");
            return 0;
        }
    }
}