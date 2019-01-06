#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Bhazel/Layer.h"

namespace BZ {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application::Application() {
        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BIND_EVENT_FN(onEvent));
    }

    void Application::run() {
        while(running) {

			for (Layer *layer : layerStack) {
				layer->onUpdate();
			}

            window->onUpdate();
        }
    }

    void Application::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
        
        BZ_CORE_TRACE("{0}", e);

		for (auto it = layerStack.end(); it != layerStack.begin(); ) {
			(*--it)->onEvent(e);
			if (e.handled) break;
		}
    }

	void Application::pushLayer(Layer *layer) {
		layerStack.pushLayer(layer);
	}

	void Application::pushOverlay(Layer *overlay) {
		layerStack.pushOverlay(overlay);
	}

    bool Application::onWindowClose(WindowCloseEvent &e) {
        running = false;
        return true;
    }
}