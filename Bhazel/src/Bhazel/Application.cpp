#include "bzpch.h"

#include "Application.h"
#include "Bhazel/Core/Timer.h"

#include "Window.h"
#include "Input.h"
#include "Events/ApplicationEvent.h"
#include "Bhazel/Layer.h"
#include "Input.h"
#include "Renderer/Renderer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Window.h"


namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        BZ_ASSERT_CORE(!instance, "Application already exists")
        instance = this;

        //Init logger
        Log::getInstance();

        //imGuiLayer = new ImGuiLayer();
        //pushOverlay(imGuiLayer);
    }

    void Application::run() {
        window = std::unique_ptr<Window>(Window::create(BZ_BIND_EVENT_FN(Application::onEvent)));
        Renderer::init();
        Input::init(window->getNativeWindowHandle());
        layerStack.onGraphicsContextCreated();

#ifndef BZ_DIST
        std::stringstream sstream;
        uint64 acumTime = 0;
#endif

        while(running) {

            Timestep timestep = timer.getAsTimestep();

#ifndef BZ_DIST
            uint64 ns = timer.getElapsedNanoseconds();
            acumTime += ns;
            if(acumTime > 250'000'000) {
                acumTime = 0;
                sstream.str(std::string());
                sstream.clear();
                sstream << window->getBaseTitle() << " | FPS: " << (1'000'000'000.0f / ns) << ". Millis: " << (ns / 1'000'000.0f);
                window->setTitle(sstream.str());
            }
#endif

            timer.start();

            for (Layer *layer : layerStack) {
                layer->onUpdate(timestep);
            }

            /*imGuiLayer->begin();
            for(Layer *layer : layerStack) {
                layer->onImGuiRender();
            }
            imGuiLayer->end();*/


            window->onUpdate();
        }

        Renderer::destroy();
    }

    void Application::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BZ_BIND_EVENT_FN(Application::onWindowClose));

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