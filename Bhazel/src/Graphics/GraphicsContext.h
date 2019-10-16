#pragma once

#include "Graphics/GraphicsApi.h"


namespace BZ {

    class WindowResizedEvent;

    class GraphicsContext {
    public:
        static GraphicsContext* create(void *windowHandle);

        virtual void init() = 0;

        virtual ~GraphicsContext() = default;
        
        virtual void onWindowResize(WindowResizedEvent& e) {};

        virtual void presentBuffer() = 0;

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        virtual Ref<Framebuffer> getCurrentFrameFramebuffer() = 0;

        GraphicsApi& getGraphicsAPI() { return *graphicsApi; }

    protected:
        GraphicsContext() = default;

        bool vsync = true;
        std::unique_ptr<GraphicsApi> graphicsApi;
    };
}