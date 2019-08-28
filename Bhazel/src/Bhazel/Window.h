#pragma once

#include "Renderer/GraphicsContext.h"


namespace BZ {

    class Event;

    struct WindowData {

        std::string title;
        uint32 width;
        uint32 height;

        WindowData(const std::string &title = "Bhazel Engine",
                    uint32 width = 1280,
                    uint32 height = 800)
            : title(title), width(width), height(height) {
        }
    };


    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        explicit Window(EventCallbackFn eventCallback, const WindowData &data);
        virtual ~Window() = default;

        EventCallbackFn eventCallback;
        WindowData data;

        virtual void onUpdate() = 0;
        virtual void* getNativeWindowHandle() const = 0;

        uint32 getWidth() const { return data.width; }
        uint32 getHeight() const {return data.height;}
        
        void setBaseTitle(const std::string &title) { data.title = title; }
        const std::string& getBaseTitle() const {return data.title;}
        
        virtual void setTitle(const std::string &title) = 0;

        GraphicsContext& getGraphicsContext() { return *graphicsContext; }

        static Window* create(EventCallbackFn eventCallback = [](Event&){}, const WindowData &data = WindowData());

    protected:
        std::unique_ptr<GraphicsContext> graphicsContext;
    };
}
