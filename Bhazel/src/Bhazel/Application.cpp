#include "bzpch.h"

#include "Application.h"

#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Log.h"

namespace BZ {

    Application::Application() {
        window = std::unique_ptr<Window>(Window::create());
    }

    Application::~Application() {
    }

    void Application::run() {
        while(running) {
            window->onUpdate();
        }
    }
}