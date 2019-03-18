#pragma once

#include "bzpch.h"

#include "Bhazel/Core.h"
#include "Bhazel/Window.h"
#include "Bhazel/LayerStack.h"

#include "Bhazel/ImGui/ImGuiLayer.h"

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

        inline Window& getWindow() { return *window; }
        inline static Application& getInstance() { return *instance; }
    private:
        bool onWindowClose(WindowCloseEvent &e);

        std::unique_ptr<Window> window;
        ImGuiLayer* imGuiLayer;
        bool running = true;

		LayerStack layerStack;

        static Application *instance;
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}