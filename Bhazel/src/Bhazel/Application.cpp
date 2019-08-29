#include "bzpch.h"

#include "Application.h"
#include "Bhazel/Core/Timer.h"
#include "Bhazel/Core/Ini/IniParser.h"

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
        //Init logger
        Log::getInstance();

        BZ_ASSERT_CORE(!instance, "Application already exists")
            instance = this;

        BZ_ASSERT_CORE(iniParser.parse("bhazel.ini"), "Failed to open \"bhazel.ini\" file.");

        //imGuiLayer = new ImGuiLayer();
        //pushOverlay(imGuiLayer);
    }

    void Application::run() {
        auto &settings = iniParser.getParsedIniSettings();
        WindowData windowData;
        windowData.width = settings.getFieldAsBasicType<uint32>("width", 1280);
        windowData.height = settings.getFieldAsBasicType<uint32>("height", 800);
        windowData.title = settings.getFieldAsString("title", "Bhazel Engine");
        //windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);
        windowData.vsync = settings.getFieldAsBasicType<bool>("vsync", true);
        
        std::string renderingAPIString = settings.getFieldAsString("renderingAPI", "");
        if(renderingAPIString == "OpenGL" || renderingAPIString == "GL") {
            Renderer::api = Renderer::API::OpenGL;
        }
        else if(renderingAPIString == "D3D" || renderingAPIString == "D3D11") {
            Renderer::api = Renderer::API::D3D11;
        }
        else {
            BZ_ASSERT_ALWAYS("Invalid Rendring API on .ini file: {0}.", renderingAPIString);
        }
        
        window = std::unique_ptr<Window>(Window::create(windowData, BZ_BIND_EVENT_FN(Application::onEvent)));
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