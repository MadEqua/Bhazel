#pragma once

#include "Bhazel/Window.h"

struct GLFWwindow;


namespace BZ {
    
    class GlfwWindow : public Window {
    public:
        explicit GlfwWindow(const WindowData &data, EventCallbackFn eventCallback);
        ~GlfwWindow() override;

        virtual void pollEvents() override;

        virtual void* getNativeWindowHandle() const override { return window; }
        virtual void setTitle(const char* title) override;

    private:
        void init();
        void shutdown();

        GLFWwindow* window;
    };
}