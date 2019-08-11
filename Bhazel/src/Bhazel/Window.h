#pragma once


namespace BZ {

    class Event;

    struct WindowProps {
        std::string title;
        uint32 width;
        uint32 height;

        WindowProps(const std::string &title = "Bhazel Engine",
                    uint32 width = 1280,
                    uint32 height = 720)
            : title(title), width(width), height(height) {}
    };

    //Interface representing a desktop based Window
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void onUpdate() = 0;

        virtual uint32 getWidth() const = 0;
        virtual uint32 getHeight() const = 0;

        virtual void setEventCallback(const EventCallbackFn &callback) = 0;
        virtual void setVSync(bool enabled) = 0;
        virtual bool isVSync() const = 0;

        virtual void* getNativeWindow() const = 0;

        static Window* create(const WindowProps &props = WindowProps());
    };
}
