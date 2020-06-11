#pragma once

#include "Core/Window.h"

#include "Core/Ini/IniParser.h"
#include "Core/Input.h"
#include "Core/Timer.h"

#include "FileWatcher/FileWatcher.h"
#include "Graphics/GraphicsContext.h"
#include "Layers/LayerStack.h"
#include "Renderer/RendererCoordinator.h"


namespace BZ {

class Event;
class WindowResizedEvent;
class Layer;
class Application;


struct FrameTiming {
    // Time since the last update.
    TimeDuration deltaTime;

    // Cummulative/total running time.
    TimeDuration runningTime;
};


class Engine {
  public:
    BZ_NON_COPYABLE(Engine);

    Engine();
    ~Engine();

    void mainLoop();
    void stopMainLoop();

    void onEvent(Event &ev);

    void attachApplication(Application *application);

    Window &getWindow() { return window; }
    GraphicsContext &getGraphicsContext() { return graphicsContext; }

    const std::string &getAssetsPath() const { return assetsPath; }

    void enable3dRenderer(bool enable) { rendererCoordinator.enable3dRenderer(enable); }
    void enable2dRenderer(bool enable) { rendererCoordinator.enable2dRenderer(enable); }
    void enableImGuiRenderer(bool enable) { rendererCoordinator.enableImGuiRenderer(enable); }

#ifdef BZ_HOT_RELOAD_SHADERS
    FileWatcher &getFileWatcher() { return fileWatcher; }
#endif

    static Engine &get() { return *instance; }

  private:
    bool onWindowResized(const WindowResizedEvent &e);

    static Engine *instance;
    Application *application;

    Window window;
    GraphicsContext graphicsContext;
    RendererCoordinator rendererCoordinator;

    IniParser iniParser;
    FrameTiming frameTiming;

    std::string assetsPath;

#ifdef BZ_HOT_RELOAD_SHADERS
    FileWatcher fileWatcher;
#endif

    bool onMainLoop = false;
    bool forceStopLoop = false;
};


/*-------------------------------------------------------------------------------------------*/
class Application {
  public:
    Application() = default;
    virtual ~Application();

    BZ_NON_COPYABLE(Application);

    void onEvent(Event &ev);

    void onAttachToEngine();
    void onUpdate(const FrameTiming &frameTiming);
    void onImGuiRender(const FrameTiming &frameTiming);

    void pushLayer(Layer *layer);
    void pushOverlay(Layer *overlay);

  private:
    LayerStack layerStack;
};
}