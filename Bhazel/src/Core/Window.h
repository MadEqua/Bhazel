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
        static Window* create(const WindowData &data, EventCallbackFn eventCallback = [](Event&) {});

        virtual ~Window() = default;

        EventCallbackFn eventCallback;
        WindowData data;

        virtual void pollEvents() = 0;
        virtual void* getNativeHandle() const = 0;

        uint32 getWidth() const { return data.dimensions.x; }
        uint32 getHeight() const {return data.dimensions.y;}
        const glm::ivec2& getDimensions() const { return data.dimensions; }
        const glm::vec2 getDimensionsFloat() const { return { data.dimensions.x, data.dimensions.y }; }
        float getRatio() const { return static_cast<float>(data.dimensions.x) / static_cast<float>(data.dimensions.y); }

        bool isMinimized() const { return minimized; }
        bool isClosed() const { return closed; }
        
        void setBaseTitle(const char* title) { data.title = title; }
        const std::string& getBaseTitle() const {return data.title;}
        
        virtual void setTitle(const char* title) = 0;

    protected:
        Window(const WindowData &data, EventCallbackFn eventCallback);

        bool minimized = false;
        bool closed = false;
    };
}
