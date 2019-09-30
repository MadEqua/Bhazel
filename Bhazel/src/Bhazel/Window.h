#pragma once


namespace BZ {

    class Event;

    struct WindowData {
        std::string title;
        uint32 width;
        uint32 height;
        //bool fullScreen;
    };


    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
        static Window* create(const WindowData &data, EventCallbackFn eventCallback = [](Event&) {});

        virtual ~Window() = default;

        EventCallbackFn eventCallback;
        WindowData data;

        virtual void pollEvents() = 0;
        virtual void* getNativeWindowHandle() const = 0;

        uint32 getWidth() const { return data.width; }
        uint32 getHeight() const {return data.height;}
        
        void setBaseTitle(const char* title) { data.title = title; }
        const std::string& getBaseTitle() const {return data.title;}
        
        virtual void setTitle(const char* title) = 0;

    protected:
        Window(const WindowData &data, EventCallbackFn eventCallback);
    };
}
