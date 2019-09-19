#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Input.h"
#include "Layer.h"
#include "Input.h"
#include "Events/ApplicationEvent.h"
#include "Renderer/Renderer.h"
#include "ImGui/ImGuiLayer.h"
#include "FrameStatsLayer.h"


namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        //Init logger
        Log::getInstance();

        BZ_ASSERT_CORE(!instance, "Application already exists");
        instance = this;

        BZ_ASSERT_CORE(iniParser.parse("bhazel.ini"), "Failed to open \"bhazel.ini\" file.");

        imGuiLayer = new ImGuiLayer();
        pushOverlay(imGuiLayer);

        pushOverlay(new FrameStatsLayer(*this));
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
            BZ_ASSERT_ALWAYS_CORE("Invalid Rendering API on .ini file: {0}.", renderingAPIString);
        }

        window = std::unique_ptr<Window>(Window::create(windowData, BZ_BIND_EVENT_FN(Application::onEvent)));
        Renderer::init();
        Input::init(window->getNativeWindowHandle());
        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameStats = {};

        while(running) {

            auto frameDuration = frameTimer.getElapsedTime();
            frameStats.lastFrameTime = frameDuration;
            frameStats.runningTime += frameDuration;

            frameTimer.start();

            for (Layer *layer : layerStack) {
                layer->onUpdate(frameStats);
            }

            imGuiLayer->begin();
            for(Layer *layer : layerStack) {
                layer->onImGuiRender(frameStats);
            }
            imGuiLayer->end();

            window->onUpdate();

            frameStats.frameCount++;
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