#pragma once

struct GLFWwindow;


namespace BZ {

    class Event;

    struct WindowData {
        std::string title;
        glm::uvec2 dimensions;
        //bool fullScreen;
    };


    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window() = default;

        BZ_NON_COPYABLE(Window);

        void init(const WindowData &data, EventCallbackFn eventCallback);
        void destroy();

        void pollEvents();
        GLFWwindow* getNativeHandle() const { return window; }

        uint32 getWidth() const { return data.dimensions.x; }
        uint32 getHeight() const {return data.dimensions.y;}
        const glm::uvec2& getDimensions() const { return data.dimensions; }
        glm::ivec2 getDimensionsInt() const { return { static_cast<int>(data.dimensions.x), static_cast<int>(data.dimensions.y) }; }
        glm::vec2 getDimensionsFloat() const { return { static_cast<float>(data.dimensions.x), static_cast<float>(data.dimensions.y) }; }
        float getAspectRatio() const { return static_cast<float>(data.dimensions.x) / static_cast<float>(data.dimensions.y); }

        bool isMinimized() const { return minimized; }
        bool isClosed() const { return closed; }

        void setTitle(const char* title);

        EventCallbackFn eventCallback;
        WindowData data;

    private:
        bool initialized = false;

        bool minimized = false;
        bool closed = false;

        GLFWwindow* window;
    };
}
