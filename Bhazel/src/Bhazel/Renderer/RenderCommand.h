#pragma once

#include "RendererAPI.h"


namespace BZ {

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

        static void drawIndexed(const Ref<InputDescription> &inputDescription) {
            rendererAPI->drawIndexed(inputDescription);
        }

    private:
        static RendererAPI *rendererAPI;
    };
}
