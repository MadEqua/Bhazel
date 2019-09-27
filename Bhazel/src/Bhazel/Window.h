#pragma once

#include "Renderer/GraphicsContext.h"


namespace BZ {

    class Event;

    struct WindowData {
        std::string title;
        uint32 width;
        uint32 height;
        //bool fullScreen;
        bool vsync;
        //uint32 MSAASamples;
    };


    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
        static Window* create(const WindowData &data, EventCallbackFn eventCallback = [](Event&) {});

        virtual ~Window() = default;

        EventCallbackFn eventCallback;
        WindowData data;

        virtual void pollEvents() = 0;
        virtual void presentBuffer() = 0;
        virtual void* getNativeWindowHandle() const = 0;

        uint32 getWidth() const { return data.width; }
        uint32 getHeight() const {return data.height;}
        
        void setBaseTitle(const std::string &title) { data.title = title; }
        const std::string& getBaseTitle() const {return data.title;}
        
        virtual void setTitle(const std::string &title) = 0;

        GraphicsContext& getGraphicsContext() { return *graphicsContext; }

    protected:
        std::unique_ptr<GraphicsContext> graphicsContext;

        Window(const WindowData &data, EventCallbackFn eventCallback);
    };
}
