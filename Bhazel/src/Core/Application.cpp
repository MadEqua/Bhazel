#include "bzpch.h"

#include "Application.h"

#include "Core/Window.h"
#include "Core/Input.h"
#include "Layers/Layer.h"
#include "Events/WindowEvent.h"
#include "ImGui/ImGuiLayer.h"
#include "Layers/FrameStatsLayer.h"


namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        //Init logger
        Log::get();

        BZ_ASSERT_CORE(!instance, "Application already exists");
        instance = this;

        BZ_ASSERT_CORE(iniParser.parse("bhazel.ini"), "Failed to open 'bhazel.ini' file.");
        auto &settings = iniParser.getParsedIniSettings();
        assetsPath = settings.getFieldAsString("assetsPath", "");

        //imGuiLayer = new ImGuiLayer();
        //pushOverlay(imGuiLayer);

        pushOverlay(new FrameStatsLayer(*this));
    }

    void Application::run() {
        auto &settings = iniParser.getParsedIniSettings();
        WindowData windowData;
        windowData.dimensions.x = settings.getFieldAsBasicType<uint32>("width", 1280);
        windowData.dimensions.y = settings.getFieldAsBasicType<uint32>("height", 800);
        windowData.title = settings.getFieldAsString("title", "Bhazel Engine Application");
        //windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);

        window = std::make_unique<Window>(windowData, BZ_BIND_EVENT_FN(Application::onEvent));
        graphicsContext = std::unique_ptr<GraphicsContext>(GraphicsContext::create(window->getNativeHandle()));
        graphicsContext->setVSync(settings.getFieldAsBasicType<bool>("vsync", true));
        graphicsContext->init();
        Input::init(window->getNativeHandle());

        Renderer::init();
        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameStats = {};

        while(!window->isClosed()) {
            frameTimer.start();

            window->pollEvents();

            if(!window->isMinimized()) {

                Renderer::startFrame();
                layerStack.onUpdate(frameStats);

                /*imGuiLayer->begin();
                layerStack.onImGuiRender(frameStats);
                imGuiLayer->end();*/

                Renderer::endFrame();
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
        if(!e.isInCategory(EventCategory::EventCategoryMouse))
            BZ_LOG_CORE_TRACE(e);

        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizedEvent>(BZ_BIND_EVENT_FN(Application::onWindowResized));

        layerStack.onEvent(e);
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