#pragma once

#include "Bhazel/Window.h"
#include <memory>

struct GLFWwindow;


namespace BZ {
    
    class GlfwWindow : public Window {
    public:
        explicit GlfwWindow(const WindowData &data, EventCallbackFn eventCallback);
        ~GlfwWindow() override;

        virtual void pollEvents() override;
        virtual void presentBuffer() override;

        virtual void* getNativeWindowHandle() const override { return window; }
        virtual void setTitle(const std::string &title) override;

    private:
        void init();
        void shutdown();

        GLFWwindow* window;
    };
}