#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Bhazel/Layer.h"
#include "Input.h"

namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        BZ_CORE_ASSERT(!instance, "Application already exists")
        instance = this;

        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BZ_BIND_EVENT_FN(Application::onEvent));
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
        dispatcher.dispatch<WindowCloseEvent>(BZ_BIND_EVENT_FN(Application::onWindowClose));
        
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