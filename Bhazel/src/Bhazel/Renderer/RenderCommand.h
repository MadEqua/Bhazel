#pragma once

#include "RendererAPI.h"


namespace BZ {

    enum class RenderMode;

    class RenderCommand
    {
    public:
        static void initRendererAPI(RendererAPI *api);

        static void setClearColor(const glm::vec4& color) {
            rendererAPI->setClearColor(color);
        }

        static void setDepthClearValue(float value) {
            rendererAPI->setDepthClearValue(value);
        }

        static void setStencilClearValue(int value) {
            rendererAPI->setStencilClearValue(value);
        }

        static void clearColorAndDepthStencilBuffers() {
            rendererAPI->clearColorAndDepthStencilBuffers();
        }

        static void setViewport(int left, int top, int width, int height) {
            rendererAPI->setViewport(left, top, width, height);
        }

        static void setRenderMode(RenderMode mode) {
            rendererAPI->setRenderMode(mode);
        }

        static void drawIndexed(const Ref<InputDescription> &inputDescription) {
            rendererAPI->drawIndexed(inputDescription);
        }

    private:
        static RendererAPI *rendererAPI;
    };
}
