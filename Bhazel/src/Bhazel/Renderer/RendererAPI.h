#pragma once

#include "Renderer.h"


namespace BZ {

    class InputDescription;

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual void setClearColor(const glm::vec4 &color) = 0;
        virtual void setDepthClearValue(float value) = 0;
        virtual void setStencilClearValue(int value) = 0;

        virtual void clearColorAndDepthStencilBuffers() = 0;

        virtual void setViewport(int left, int top, int width, int height) = 0;
        virtual void setRenderMode(Renderer::RenderMode mode) = 0;

        virtual void drawIndexed(const Ref<InputDescription> &inputDesc) = 0;
    };
}