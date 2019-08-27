#include "bzpch.h"

#include <Glad/glad.h>
#include "OpenGLRendererAPI.h"


namespace BZ {

    void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::setDepthClearValue(float value) {
        glClearDepthf(value);
    }

    void OpenGLRendererAPI::setStencilClearValue(int value) {
        glClearStencil(value);
    }

    void OpenGLRendererAPI::clearColorAndDepthStencilBuffers() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRendererAPI::drawIndexed(const Ref<InputDescription> &inputDesc) {
        glDrawElements(GL_TRIANGLES, inputDesc->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
    }
}