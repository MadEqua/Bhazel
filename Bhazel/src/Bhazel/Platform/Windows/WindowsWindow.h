#pragma once

#include "Bhazel/Window.h"

struct GLFWwindow;

namespace BZ {
    
    class WindowsWindow : public Window {
    public:
        WindowsWindow(const WindowProps& props);
        virtual ~WindowsWindow() override;

        void onUpdate() override;

        inline unsigned int getWidth() const override { return data.width; }
        inline unsigned int getHeight() const override { return data.height; }

        inline void setEventCallback(const EventCallbackFn& callback) override { data.eventCallback = callback; }
        void setVSync(bool enabled) override;
        bool isVSync() const override;

        inline void* getNativeWindow() const override { return window; }

    private:
        virtual void init(const WindowProps& props);
        virtual void shutdown();

        GLFWwindow* window;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            bool vSync;

            EventCallbackFn eventCallback;
        };

        WindowData data;
    };
}