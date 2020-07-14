#pragma once

#include "Core/Window.h"

#include "Core/Ini/IniParser.h"
#include "Core/Input.h"
#include "Core/Timer.h"
#include "FileWatcher/FileWatcher.h"
#include "Graphics/GraphicsContext.h"
#include "Renderer/RendererCoordinator.h"
#include "Scene/SceneManager.h"


namespace BZ {

class Event;
class WindowResizedEvent;
class Layer;
class Application;
class EcsInstance;
class Scene;


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
    RendererCoordinator &getRendererCoordinator() { return rendererCoordinator; }

    const std::string &getAssetsPath() const { return assetsPath; }

#ifdef BZ_HOT_RELOAD_SHADERS
    FileWatcher &getFileWatcher() { return fileWatcher; }
#endif

    static Engine &get() { return *instance; }

  private:
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

    void updateSystems(const FrameTiming &frameTiming, Scene &scene);
};


/*-------------------------------------------------------------------------------------------*/
class Application {
  public:
    struct Settings {
        // If true, all renderers are enabled and the 2D and 3D renderers will use default Swapchain while ImGui
        // renderer will use an offscreen Framebuffer.
        bool editorMode = false;

        bool enable2dRenderer = true;
        bool enable3dRenderer = true;
        bool enableImGuiRenderer = true;
    };

    Application() = default;
    virtual ~Application() = default;

    BZ_NON_COPYABLE(Application);

    void onAttachToEngine();
    void onUpdate(const FrameTiming &frameTiming);
    void onImGuiRender(const FrameTiming &frameTiming);
    void onEvent(Event &ev);

    const Settings &getSettings() const { return settings; }
    SceneManager &getSceneManager() { return sceneManager; }

  protected:
    SceneManager sceneManager;

    Settings settings;
};
}