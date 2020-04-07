#include "bzpch.h"

#include "Core/Application.h"

#include "Window.h"
#include "Layers/Layer.h"
#include "Events/WindowEvent.h"
#include "Graphics/Graphics.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer.h"
#include "Renderer/ImGuiRenderer.h"
#include "Layers/FrameStatsLayer.h"
#include "Layers/Renderer2DStatsLayer.h"
#include "Layers/RendererStatsLayer.h"


namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(!instance, "Application already exists");
        instance = this;

        BZ_CRITICAL_ERROR_CORE(iniParser.parse("bhazel.ini"), "Failed to open \"bhazel.ini\" file.");
        auto &settings = iniParser.getParsedIniSettings();
        std::string renderingAPIString = settings.getFieldAsString("renderingAPI", "");
        if(renderingAPIString == "OpenGL" || renderingAPIString == "GL") 
            Graphics::api = Graphics::API::OpenGL;
        else if(renderingAPIString == "D3D" || renderingAPIString == "D3D11") 
            Graphics::api = Graphics::API::D3D11;
        else if(renderingAPIString == "VK" || renderingAPIString == "Vulkan") 
            Graphics::api = Graphics::API::Vulkan;
        else 
            BZ_CRITICAL_ERROR_CORE("Invalid Rendering API on .ini file: {0}.", renderingAPIString);

        assetsPath = settings.getFieldAsString("assetsPath", "");

        WindowData windowData;
        windowData.dimensions.x = settings.getFieldAsBasicType<uint32>("width", 1280);
        windowData.dimensions.y = settings.getFieldAsBasicType<uint32>("height", 800);
        windowData.title = settings.getFieldAsString("title", "Bhazel Engine Application");
        //windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);

        window = std::unique_ptr<Window>(Window::create(windowData, BZ_BIND_EVENT_FN(Application::onEvent)));
        graphicsContext = std::unique_ptr<GraphicsContext>(GraphicsContext::create(window->getNativeHandle()));
        graphicsContext->setVSync(settings.getFieldAsBasicType<bool>("vsync", true));
        graphicsContext->init();
        input = std::unique_ptr<Input>(Input::create(window->getNativeHandle()));

        Graphics::init();
        ImGuiRenderer::init();
        Renderer2D::init();
        Renderer::init();

        pushOverlay(new FrameStatsLayer(*this));
        pushOverlay(new Renderer2DStatsLayer());
        pushOverlay(new RendererStatsLayer());

#ifdef BZ_HOT_RELOAD_SHADERS
        fileWatcher.startWatching();
#endif
    }

    Application::~Application() {
        BZ_PROFILE_FUNCTION();

        Graphics::waitForDevice();
        Renderer::destroy();
        Renderer2D::destroy();
        ImGuiRenderer::destroy();
        Graphics::destroy();
    }

    void Application::run() {
        BZ_PROFILE_FUNCTION();

        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameStats = {};

        while(!window->isClosed()) {
            frameTimer.start();

            window->pollEvents();

            if(!window->isMinimized()) {

                Graphics::beginFrame();

                layerStack.onUpdate(frameStats);

                ImGuiRenderer::begin();
                layerStack.onImGuiRender(frameStats);
                ImGuiRenderer::end();

                Graphics::endFrame();

                auto frameDuration = frameTimer.getCountedTime();
                frameStats.lastFrameTime = frameDuration;
                frameStats.runningTime += frameDuration;
                frameStats.frameCount++;

#ifdef BZ_HOT_RELOAD_SHADERS
                if (fileWatcher.hasPipelineStatesToReload()) {
                    Graphics::waitForDevice();
                    fileWatcher.performReloads();
                }
#endif
            }
            frameTimer.reset();
        }
    }

    void Application::onEvent(Event &e) {
        //BZ_LOG_CORE_TRACE(e);

        ImGuiRenderer::onEvent(e);

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

    bool Application::onWindowResized(const WindowResizedEvent &e) {
        graphicsContext->onWindowResize(e);
        Graphics::onWindowResize(e);
        return false;
    }
}