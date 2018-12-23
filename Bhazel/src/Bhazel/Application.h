#pragma once

#include "bzpch.h"

#include "Core.h"

namespace BZ {

    class Window;
    class Event;
    class WindowCloseEvent;

    class BZ_API Application
    {
    public:
        Application();
        ~Application();

        void run();
        
        void onEvent(Event &ev);
    private:
        bool onWindowClose(WindowCloseEvent &e);

        std::unique_ptr<Window> window;
        bool running = true;
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}