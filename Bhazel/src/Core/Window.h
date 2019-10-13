#pragma once


namespace BZ {

    class Event;

    struct WindowData {
        std::string title;
        glm::ivec2 dimensions;
        //bool fullScreen;
    };


    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const WindowData &data, EventCallbackFn eventCallback);

        void pollEvents();
        void *getNativeHandle() const { return nativeHandle; }

        uint32 getWidth() const { return data.dimensions.x; }
        uint32 getHeight() const {return data.dimensions.y;}
        const glm::ivec2& getDimensions() const { return data.dimensions; }

        bool isMinimized() const { return minimized; }
        bool isClosed() const { return closed; }
        
        void setBaseTitle(const char* title) { data.title = title; }
        const std::string& getBaseTitle() const {return data.title;}

        void setTitle(const char* title);

    private:
        EventCallbackFn eventCallback;
        WindowData data;

        bool minimized = false;
        bool closed = false;

        void *nativeHandle = nullptr;
    };
}
