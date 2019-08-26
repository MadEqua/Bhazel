#pragma once

#include "Renderer/GraphicsContext.h"


namespace BZ {

    class Event;

    struct WindowData {
        using EventCallbackFn = std::function<void(Event&)>;

        std::string title;
        uint32 width;
        uint32 height;
        EventCallbackFn eventCallback;

        WindowData(const std::string &title = "Bhazel Engine",
                    uint32 width = 1280,
                    uint32 height = 800)
            : title(title), width(width), height(height), eventCallback([](Event&) {}) {
        }
    };


    class Window {
    public:

        virtual ~Window() = default;

        virtual void onUpdate() = 0;
        virtual void* getNativeWindowHandle() const = 0;

        void setEventCallback(const WindowData::EventCallbackFn &callback) { windowData.eventCallback = callback; }

        uint32 getWidth() const { return windowData.width; }
        uint32 getHeight() const {return windowData.height;}
        const std::string& getTitle() const {return windowData.title;}

        GraphicsContext& getGraphicsContext() { return *graphicsContext; }

        static Window* create(const WindowData &data = WindowData());

    protected:
        WindowData windowData;
        std::unique_ptr<GraphicsContext> graphicsContext;
    };
}
