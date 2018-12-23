#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace BZ {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application::Application() {
        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BIND_EVENT_FN(onEvent));
    }

    Application::~Application() {
    }

    void Application::run() {
        while(running) {
            window->onUpdate();
        }
    }

    void Application::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
        
        BZ_CORE_TRACE("{0}", e);
    }

    bool Application::onWindowClose(WindowCloseEvent &e) {
        running = false;
        return true;
    }
}