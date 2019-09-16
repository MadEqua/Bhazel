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

    void OpenGLInputDescription::addVertexBuffer(const Ref<Buffer> &buffer, const Ref<Shader> &vertexShader) {
        //TODO use the vertex shader to do some validations

        BZ_ASSERT_CORE(buffer->getLayout().getElementCount(), "VertexBuffer has no layout.");

        BZ_ASSERT_GL(glBindVertexArray(rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ARRAY_BUFFER, static_cast<OpenGLBuffer*>(buffer.get())->rendererId));

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

    void OpenGLInputDescription::setIndexBuffer(const Ref<Buffer> &buffer) {
        BZ_ASSERT_GL(glBindVertexArray(rendererId));
        BZ_ASSERT_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<OpenGLBuffer*>(buffer.get())->rendererId));

        indexBuffer = buffer;
    }

    GLenum OpenGLInputDescription::shaderDataTypeToGL(DataType dataType) {
        switch(dataType)
        {
        case DataType::Float:
        case DataType::Vec2:
        case DataType::Vec3:
        case DataType::Vec4:
        case DataType::Mat2:
        case DataType::Mat3:
        case DataType::Mat4:
            return GL_FLOAT;
        case DataType::Int:
        case DataType::Vec2i:
        case DataType::Vec3i:
        case DataType::Vec4i:
            return GL_INT;
        case DataType::Uint:
        case DataType::Vec2ui:
        case DataType::Vec3ui:
        case DataType::Vec4ui:
            return GL_UNSIGNED_INT;
        case DataType::Int16:
            return GL_SHORT;
        case DataType::Int8:
            return GL_BYTE;
        case DataType::Uint16:
            return GL_UNSIGNED_SHORT;
        case DataType::Uint8:
            return GL_UNSIGNED_BYTE;
        case DataType::Bool:
            return GL_BOOL;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return 0;
        }
    }
}