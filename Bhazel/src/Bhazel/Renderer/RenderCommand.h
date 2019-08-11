#pragma once

#include <memory>

#include "RendererAPI.h"


namespace BZ {

    class RenderCommand
    {
    public:
        static void drawIndexed(const Ref<VertexArray> &vertexArray) {
            rendererAPI->drawIndexed(vertexArray);
        }

        static void setClearColor(const glm::vec4& color) {
            rendererAPI->setClearColor(color);
        }

        static void clear() {
            rendererAPI->clear();
        }

    private:
        static RendererAPI *rendererAPI;
    };
}
