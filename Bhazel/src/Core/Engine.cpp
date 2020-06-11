#include "bzpch.h"

#include "Engine.h"

#include "Events/WindowEvent.h"
#include "Layers/Layer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RendererImGui.h"

#include <GLFW/glfw3.h>


namespace BZ {

static void GLFWErrorCallback(int error, const char *description) {
    BZ_LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
}

Engine* Engine::instance = nullptr;

Engine::Engine() {
    BZ_PROFILE_FUNCTION();

    BZ::Log::get(); // Init Logger.

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
    windowData.title = settings.getFieldAsString("title", "Bhazel Engine Untitled Engine");
    // windowData.fullScreen = settings.getFieldAsBasicType<bool>("fullScreen", false);

    window.init(windowData, BZ_BIND_EVENT_FN(Engine::onEvent));
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

Engine::~Engine() {
    BZ_PROFILE_FUNCTION();

    BZ_LOG_CORE_INFO("Shutting down Engine.");
    graphicsContext.waitForDevice();

    delete application;

    RendererImGui::destroy();
    Renderer2D::destroy();
    Renderer::destroy();

    rendererCoordinator.destroy();

    graphicsContext.destroy();
    window.destroy();

    glfwTerminate();
}

void Engine::attachApplication(Application *application) {
    BZ_ASSERT_CORE(application, "Invalid Application!");
    this->application = application;

    application->onAttachToEngine();
}

void Engine::mainLoop() {
    BZ_PROFILE_FUNCTION();

    BZ_ASSERT_CORE(!onMainLoop, "Engine is already on main loop!");
    onMainLoop = true;

    Timer frameTimer;
    frameTiming = {};

    while (!window.isClosed() || forceStopLoop) {

        window.pollEvents();

        if (!window.isMinimized()) {

            graphicsContext.beginFrame();

            auto frameDuration = frameTimer.getCountedTime();
            frameTimer.restart();
            frameTiming.deltaTime = frameDuration;
            frameTiming.runningTime += frameDuration;
            application->onUpdate(frameTiming);

            RendererImGui::begin();
            graphicsContext.onImGuiRender(frameTiming);
            Renderer::onImGuiRender(frameTiming);
            Renderer2D::onImGuiRender(frameTiming);
            application->onImGuiRender(frameTiming);
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

void Engine::stopMainLoop() {
    forceStopLoop = true;
}

void Engine::onEvent(Event &e) {
    // BZ_LOG_CORE_TRACE(e);

    rendererCoordinator.onEvent(e);

    RendererImGui::onEvent(e);

    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowResizedEvent>([this](const WindowResizedEvent &e) -> bool {
        graphicsContext.onWindowResize(e);
        return false;
    });

    application->onEvent(e);
}


/*-------------------------------------------------------------------------------------------*/
Application::~Application() {
    layerStack.clear();
}

void Application::onAttachToEngine() {
    layerStack.onAttachToEngine();
}

void Application::onUpdate(const FrameTiming &frameTiming) {
    layerStack.onUpdate(frameTiming);
}

void Application::onImGuiRender(const FrameTiming &frameTiming) {
    layerStack.onImGuiRender(frameTiming);
}

void Application::onEvent(Event &e) {
    layerStack.onEvent(e);
}

void Application::pushLayer(Layer *layer) {
    layerStack.pushLayer(layer);
}

void Application::pushOverlay(Layer *overlay) {
    layerStack.pushOverlay(overlay);
}
}