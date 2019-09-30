#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    class GraphicsContext
    {
    public:
        static GraphicsContext* create(void *windowHandle);

        virtual ~GraphicsContext() = default;
        
        virtual void presentBuffer() = 0;

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        RendererAPI& getRendererAPI() { return *rendererAPI; }

    protected:
        bool vsync = true;
        std::unique_ptr<RendererAPI> rendererAPI;
    };
}