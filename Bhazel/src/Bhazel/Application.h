#pragma once

#include "bzpch.h"

#include "Core.h"
#include "Bhazel/Window.h"
#include "Bhazel/LayerStack.h"

namespace BZ {

	class Event;
    class WindowCloseEvent;
	class Layer;

    class BZ_API Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void run();
        
        void onEvent(Event &ev);

		void pushLayer(Layer *layer);
		void pushOverlay(Layer *overlay);
    private:
        bool onWindowClose(WindowCloseEvent &e);

        std::unique_ptr<Window> window;
        bool running = true;

		LayerStack layerStack;
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}