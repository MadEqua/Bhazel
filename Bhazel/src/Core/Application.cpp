#include "bzpch.h"

#include "Application.h"

#include "Layers/Layer.h"
#include "Events/WindowEvent.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RendererImGui.h"

#include <GLFW/glfw3.h>


namespace BZ {

    Application* Application::instance = nullptr;

    static void GLFWErrorCallback(int error, const char* description) {
        BZ_LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Application::Application() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(!instance, "Application already exists");
        instance = this;

        glfwSetErrorCallback(GLFWErrorCallback);
        int success = glfwInit();
        BZ_CRITICAL_ERROR_CORE(success, "Could not intialize GLFW!");

        BZ_CRITICAL_ERROR_CORE(iniParser.parse("bhazel.ini"), "Failed to open \"bhazel.ini\" file.");
        auto &settings = iniParser.getParsedIniSettings();
        assetsPath = settings.getFieldAsString("assetsPath", "");

        WindowData windowData;
        windowData.dimensions.x = settings.getFieldAsBasicType<uint32>("width", 1280);
        windowData.dimensions.y = settings.getFieldAsBasicType<uint32>("height", 800);
        windowData.title = settings.getFieldAsString("title", "Bhazel Engine Untitled Application");
        //windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);

        window.init(windowData, BZ_BIND_EVENT_FN(Application::onEvent));
        graphicsContext.init();

        RendererImGui::init();
        Renderer2D::init();
        Renderer::init();
        rendererCoordinator.init();

        Input::init();

#ifdef BZ_HOT_RELOAD_SHADERS
        fileWatcher.startWatching();
#endif
    }

    Application::~Application() {
        BZ_PROFILE_FUNCTION();

        graphicsContext.waitForDevice();
        RendererImGui::destroy();
        Renderer2D::destroy();
        Renderer::destroy();

        rendererCoordinator.destroy();

        layerStack.clear();

        graphicsContext.destroy();
        window.destroy();

        glfwTerminate();
    }

    void Application::run() {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(!running, "Application is already running!");
        running = true;

        layerStack.onGraphicsContextCreated();

        Timer frameTimer;
        frameTiming = {};

        while(!window.isClosed()) {

            window.pollEvents();

            if(!window.isMinimized()) {

                graphicsContext.beginFrame();

                auto frameDuration = frameTimer.getCountedTime();
                frameTimer.restart();
                frameTiming.deltaTime = frameDuration;
                frameTiming.runningTime += frameDuration;
                layerStack.onUpdate(frameTiming);

                RendererImGui::begin();
                graphicsContext.onImGuiRender(frameTiming);
                Renderer::onImGuiRender(frameTiming);
                Renderer2D::onImGuiRender(frameTiming);
                layerStack.onImGuiRender(frameTiming);
                RendererImGui::end();

                rendererCoordinator.render();

                graphicsContext.endFrame();

#ifdef BZ_HOT_RELOAD_SHADERS
                if (fileWatcher.hasPipelineStatesToReload()) {
                    graphicsContext.waitForDevice();
                    fileWatcher.performReloads();
                }
#endif
            }
        }
    }

    void Application::onEvent(Event &e) {
        //BZ_LOG_CORE_TRACE(e);

        rendererCoordinator.onEvent(e);

        RendererImGui::onEvent(e);

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
        graphicsContext.onWindowResize(e);
        return false;
    }
}