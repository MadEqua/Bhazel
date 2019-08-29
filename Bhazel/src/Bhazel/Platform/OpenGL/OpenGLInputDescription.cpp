#include "bzpch.h"

#include "OpenGLInputDescription.h"
#include "OpenGLBuffer.h"

namespace BZ {

    OpenGLInputDescription::OpenGLInputDescription() {
        BZ_ASSERT_GL(glGenVertexArrays(1, &rendererId));
    }

    OpenGLInputDescription::~OpenGLInputDescription() {
        BZ_ASSERT_GL(glDeleteVertexArrays(1, &rendererId));
    }

    void OpenGLInputDescription::bindToPipeline() const {
        BZ_ASSERT_GL(glBindVertexArray(rendererId));
    }

    void OpenGLInputDescription::unbindFromPipeline() const {
        BZ_ASSERT_GL(glBindVertexArray(0));
    }

    void OpenGLInputDescription::addVertexBuffer(const Ref<VertexBuffer> &buffer, const Ref<Shader> &vertexShader) {
        //TODO use the vertex shader to do some validations

        BZ_ASSERT_CORE(buffer->getLayout().getElementCount(), "VertexBuffer has no layout.");

        BZ_ASSERT_GL(glBindVertexArray(rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ARRAY_BUFFER, static_cast<OpenGLVertexBuffer*>(buffer.get())->rendererId));

        int index = 0;
        for(const auto &element : buffer->getLayout()) {
            BZ_ASSERT_GL(glEnableVertexAttribArray(index));
            BZ_ASSERT_GL(glVertexAttribPointer(index, element.getElementCount(),
                                  shaderDataTypeToGL(element.dataType),
                                  element.normalized ? GL_TRUE : GL_FALSE,
                                  buffer->getLayout().getStride(), reinterpret_cast<const void*>(element.offset)));
            index++;
        }

        vertexBuffers.emplace_back(buffer);
    }

    void OpenGLInputDescription::setIndexBuffer(const Ref<IndexBuffer> &buffer) {
        BZ_ASSERT_GL(glBindVertexArray(rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<OpenGLIndexBuffer*>(buffer.get())->rendererId));

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
        case ShaderDataType::Uint:
        case ShaderDataType::Vec2ui:
        case ShaderDataType::Vec3ui:
        case ShaderDataType::Vec4ui:
            return GL_UNSIGNED_INT;
        case ShaderDataType::Int16:
            return GL_SHORT;
        case ShaderDataType::Int8:
            return GL_BYTE;
        case ShaderDataType::Uint16:
            return GL_UNSIGNED_SHORT;
        case ShaderDataType::Uint8:
            return GL_UNSIGNED_BYTE;
        case ShaderDataType::Bool:
            return GL_BOOL;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown ShaderDataType.");
            return 0;
        }
    }
}