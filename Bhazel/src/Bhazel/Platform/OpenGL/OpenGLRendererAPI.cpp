#include "bzpch.h"

#include "OpenGLIncludes.h"
#include "OpenGLRendererAPI.h"

#include "Bhazel/Renderer/Renderer.h"


namespace BZ {

    void OpenGLRendererAPI::setClearColor(const glm::vec4& color) {
        BZ_ASSERT_GL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void OpenGLRendererAPI::setDepthClearValue(float value) {
        BZ_ASSERT_GL(glClearDepthf(value));
    }

    void OpenGLRendererAPI::setStencilClearValue(int value) {
        BZ_ASSERT_GL(glClearStencil(value));
    }

    void OpenGLRendererAPI::clearColorAndDepthStencilBuffers() {
        BZ_ASSERT_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    }

    void OpenGLRendererAPI::setViewport(int left, int top, int width, int height) {
        BZ_ASSERT_GL(glViewport(left, top, width, height));
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
        BZ_ASSERT_GL(glDrawElements(renderMode, inputDesc->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }
}