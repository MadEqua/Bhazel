#pragma once

#include "Core/Window.h"

#include "Core/Input.h"
#include "Core/Ini/IniParser.h"
#include "Core/Timer.h"

#include "Renderer/RendererCoordinator.h"
#include "Graphics/GraphicsContext.h"
#include "Layers/LayerStack.h"
#include "FileWatcher/FileWatcher.h"


namespace BZ {

    class Event;
    class WindowResizedEvent;
    class Layer;

    struct FrameTiming {
        //Time since the last update.
        TimeDuration deltaTime;

        //Cummulative/total running time.
        TimeDuration runningTime;
    };

    class Application {
    public:
        Application();
        virtual ~Application();

        BZ_NON_COPYABLE(Application);

        void run();
        void onEvent(Event &ev);

        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);

        Window& getWindow() { return window; }
        GraphicsContext& getGraphicsContext() { return graphicsContext; }

        const std::string& getAssetsPath() const { return assetsPath; }

        void enable3dRenderer(bool enable) { rendererCoordinator.enable3dRenderer(enable); }
        void enable2dRenderer(bool enable) { rendererCoordinator.enable2dRenderer(enable); }
        void enableImGuiRenderer(bool enable) { rendererCoordinator.enableImGuiRenderer(enable); }

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher& getFileWatcher() { return fileWatcher; }
#endif      
        static Application& get() { return *instance; }

    private:
        bool onWindowResized(const WindowResizedEvent &e);

        Window window;
        GraphicsContext graphicsContext;
        RendererCoordinator rendererCoordinator;

        LayerStack layerStack;
        IniParser iniParser;
        FrameTiming frameTiming;

        std::string assetsPath;

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher fileWatcher;
#endif

        bool running = false;
        static Application *instance;
    };
}