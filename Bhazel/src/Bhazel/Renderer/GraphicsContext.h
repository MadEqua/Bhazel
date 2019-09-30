#pragma once

#include "Bhazel/Renderer/RendererAPI.h"


namespace BZ {

    class WindowResizedEvent;

    class GraphicsContext
    {
    public:
        static GraphicsContext* create(void *windowHandle);

        virtual ~GraphicsContext() = default;
        
        virtual void onWindowResize(WindowResizedEvent& e) {};
        virtual void presentBuffer() = 0;

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        RendererAPI& getRendererAPI() { return *rendererAPI; }

    protected:
        GraphicsContext() = default;

        bool vsync = true;
        std::unique_ptr<RendererAPI> rendererAPI;
    };
}