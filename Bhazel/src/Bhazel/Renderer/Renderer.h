#pragma once

#include <memory>

#include "RenderCommand.h"


namespace BZ {

    class Renderer
    {
    public:
        static void beginScene();
        static void endScene();

        static void submit(const std::shared_ptr<VertexArray> &vertexArray);

        static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }
    };
}