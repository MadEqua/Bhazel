#pragma once

#include "RenderCommand.h"


namespace BZ {

    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;
        
        virtual void presentBuffer() = 0;

        //Called on app startup and on window resize
        virtual void onWindowResize(uint32 width, uint32 height) {}

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        RendererAPI& getRendererAPI() { return *rendererAPI; }

    protected:
        bool vsync = true;
        std::unique_ptr<RendererAPI> rendererAPI;
    };
}