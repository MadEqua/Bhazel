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
        auto &settings = iniParser.getParsedIniSettings();
        std::string renderingAPIString = settings.getFieldAsString("renderingAPI", "");
        if(renderingAPIString == "OpenGL" || renderingAPIString == "GL") Renderer::api = Renderer::API::OpenGL;
        else if(renderingAPIString == "D3D" || renderingAPIString == "D3D11") Renderer::api = Renderer::API::D3D11;
        else BZ_ASSERT_ALWAYS_CORE("Invalid Rendering API on .ini file: {0}.", renderingAPIString);

        assetsPath = settings.getFieldAsString("assetsPath", "");

        imGuiLayer = new ImGuiLayer();
        pushOverlay(imGuiLayer);

        pushOverlay(new FrameStatsLayer(*this));
    }

    void Application::run() {
        auto &settings = iniParser.getParsedIniSettings();
        WindowData windowData;
        windowData.width = settings.getFieldAsBasicType<uint32>("width", 1280);
        windowData.height = settings.getFieldAsBasicType<uint32>("height", 800);
        windowData.title = settings.getFieldAsString("title", "Bhazel Engine Application");
        //windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);
        windowData.vsync = settings.getFieldAsBasicType<bool>("vsync", true);

        window = std::unique_ptr<Window>(Window::create(windowData, BZ_BIND_EVENT_FN(Application::onEvent)));
        Renderer::init();
        Input::init(window->getNativeWindowHandle());
        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameStats = {};
        bool frameStartedMinimized;

        while(running) {
            frameTimer.start();
            frameStartedMinimized = minimized;

            window->pollEvents();

            if(!frameStartedMinimized) {

                for(Layer *layer : layerStack) {
                    layer->onUpdate(frameStats);
                }

                imGuiLayer->begin();
                for(Layer *layer : layerStack) {
                    layer->onImGuiRender(frameStats);
                }
                imGuiLayer->end();

                window->presentBuffer();

                auto frameDuration = frameTimer.getCountedTime();
                frameStats.lastFrameTime = frameDuration;
                frameStats.runningTime += frameDuration;
                frameStats.frameCount++;
            }
            frameTimer.reset();
        }

        Renderer::destroy();
    }

    void Application::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BZ_BIND_EVENT_FN(Application::onWindowClose));
        dispatcher.dispatch<WindowResizeEvent>(BZ_BIND_EVENT_FN(Application::onWindowResize));

        for (auto it = layerStack.end(); it != layerStack.begin(); ) {
            if (e.handled) break;
            (*--it)->onEvent(e);
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

    bool Application::onWindowResize(WindowResizeEvent &e) {
        if(e.getWidth() == 0 || e.getHeight() == 0) {
            minimized = true;
            return true;
        }

        Renderer::onWindowResize(e);
        minimized = false;
        return false;
    }
}