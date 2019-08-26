#pragma once

#include "Bhazel/Window.h"
#include <memory>

struct GLFWwindow;


namespace BZ {
    
    class GlfwWindow : public Window {
    public:
        explicit GlfwWindow(const WindowData& data);
        ~GlfwWindow() override;

        void onUpdate() override;

        void* getNativeWindowHandle() const override { return window; }

    private:
        void init(const WindowData& data);
        void shutdown();

        GLFWwindow* window;
    };
}