#include "bzpch.h"

#include "Application.h"

#include "Bhazel/Window.h"
#include "Bhazel/Input.h"
#include "Bhazel/Layer.h"
#include "Bhazel/Events/WindowEvent.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/ImGui/ImGuiLayer.h"
#include "Bhazel/FrameStatsLayer.h"


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
        if(renderingAPIString == "OpenGL" || renderingAPIString == "GL") 
            Renderer::api = Renderer::API::OpenGL;
        else if(renderingAPIString == "D3D" || renderingAPIString == "D3D11") 
            Renderer::api = Renderer::API::D3D11;
        else 
            BZ_ASSERT_ALWAYS_CORE("Invalid Rendering API on .ini file: {0}.", renderingAPIString);

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

        window = std::unique_ptr<Window>(Window::create(windowData, BZ_BIND_EVENT_FN(Application::onEvent)));
        graphicsContext = std::unique_ptr<GraphicsContext>(GraphicsContext::create(window->getNativeWindowHandle()));
        graphicsContext->setVSync(settings.getFieldAsBasicType<bool>("vsync", true));

        Renderer::init();
        Input::init(window->getNativeWindowHandle());
        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameStats = {};
        bool frameStartedMinimized;

        while(!window->isClosed()) {
            frameTimer.start();
            frameStartedMinimized = window->isMinimized();

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

                graphicsContext->presentBuffer();

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
        BZ_LOG_CORE_TRACE(e);

        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizedEvent>(BZ_BIND_EVENT_FN(Application::onWindowResized));

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

    bool Application::onWindowResized(WindowResizedEvent &e) {
        graphicsContext->onWindowResize(e);
        Renderer::onWindowResize(e);
        return false;
    }
}