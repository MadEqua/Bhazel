#pragma once

#include "Core.h"
#include "Window.h"

namespace BZ {

    class BZ_API Application
    {
    public:
        Application();
        ~Application();

        void run();

    private:
        std::unique_ptr<Window> window;
        bool running = true;
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}