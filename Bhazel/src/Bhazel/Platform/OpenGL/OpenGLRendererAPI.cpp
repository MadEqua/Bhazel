#include "bzpch.h"

#include <Glad/glad.h>
#include "OpenGLRendererAPI.h"

#include "Bhazel/Renderer/Renderer.h"


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

    void OpenGLRendererAPI::setViewport(int left, int top, int width, int height) {
        glViewport(left, top, width, height);
    }

    void OpenGLRendererAPI::setRenderMode(RenderMode mode) {
        switch(mode)
        {
        case BZ::RenderMode::Points:
            renderMode = GL_POINTS;
            break;
        case BZ::RenderMode::Lines:
            renderMode = GL_LINES;
            break;
        case BZ::RenderMode::Triangles:
            renderMode = GL_TRIANGLES;
            break;
        default:
            BZ_LOG_ERROR("Unknown RenderMode. Setting triangles.");
            renderMode = GL_TRIANGLES;
        }
    }

    void OpenGLRendererAPI::drawIndexed(const Ref<InputDescription> &inputDesc) {
        glDrawElements(renderMode, inputDesc->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
    }
}