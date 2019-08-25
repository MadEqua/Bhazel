#pragma once

#include "RenderCommand.h"


namespace BZ {

    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;
        virtual void swapBuffers() = 0;

        RendererAPI& getRendererAPI() { return *rendererAPI; }

    protected:
        std::unique_ptr<RendererAPI> rendererAPI;
    };
}