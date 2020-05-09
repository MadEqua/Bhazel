#pragma once

#include "Core/Window.h"

#include "Core/Input.h"
#include "Core/Ini/IniParser.h"
#include "Core/Timer.h"

#include "Graphics/GraphicsContext.h"
#include "Layers/LayerStack.h"
#include "FileWatcher/FileWatcher.h"


namespace BZ {

    class Event;
    class WindowResizedEvent;
    class Layer;

    struct FrameStats {
        TimeDuration lastFrameTime;
        uint64 frameCount;
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

        const FrameStats& getFrameStats() const { return frameStats; }
        const std::string& getAssetsPath() const { return assetsPath; }

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher& getFileWatcher() { return fileWatcher; }
#endif      
        static Application& get() { return *instance; }

    private:
        bool onWindowResized(const WindowResizedEvent &e);

        Window window;
        GraphicsContext graphicsContext;

        LayerStack layerStack;
        IniParser iniParser;
        FrameStats frameStats;

        std::string assetsPath;

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher fileWatcher;
#endif

        static Application *instance;
    };
}