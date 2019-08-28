#pragma once

#include "Bhazel/Window.h"
#include <memory>

struct GLFWwindow;


namespace BZ {
    
    class GlfwWindow : public Window {
    public:
        explicit GlfwWindow(EventCallbackFn eventCallback, const WindowData &data);
        ~GlfwWindow() override;

        void onUpdate() override;

        void* getNativeWindowHandle() const override { return window; }

    private:
        void init();
        void shutdown();

        GLFWwindow* window;
    };
}