#pragma once

#include "Bhazel/Window.h"
#include <memory>

struct GLFWwindow;

namespace BZ {

    class GraphicsContext;
    
    class WindowsWindow : public Window {
    public:
        WindowsWindow(const WindowProps& props);
        virtual ~WindowsWindow() override;

        void onUpdate() override;

        inline uint32 getWidth() const override { return data.width; }
        inline uint32 getHeight() const override { return data.height; }

        inline void setEventCallback(const EventCallbackFn& callback) override { data.eventCallback = callback; }
        void setVSync(bool enabled) override;
        bool isVSync() const override;

        inline void* getNativeWindow() const override { return window; }

    private:
        virtual void init(const WindowProps& props);
        virtual void shutdown();

        GLFWwindow* window;
        std::unique_ptr<GraphicsContext> graphicsContext;

        struct WindowData {
            std::string title;
            uint32 width, height;
            bool vSync;

            EventCallbackFn eventCallback;
        };

        WindowData data;
    };
}