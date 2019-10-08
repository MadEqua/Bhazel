#pragma once

#include "Bhazel/Renderer/RendererApi.h"


namespace BZ {

    class WindowResizedEvent;

    class GraphicsContext
    {
    public:
        static GraphicsContext* create(void *windowHandle);

        virtual void init() = 0;

        virtual ~GraphicsContext() = default;
        
        virtual void onWindowResize(WindowResizedEvent& e) {};
        virtual void presentBuffer() = 0;

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        RendererApi& getRendererAPI() { return *rendererApi; }

    protected:
        GraphicsContext() = default;

        bool vsync = true;
        std::unique_ptr<RendererApi> rendererApi;
    };
}