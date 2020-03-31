#pragma once

#include "Graphics/GraphicsContext.h"
#include "Window.h"
#include "Core/Input.h"
#include "Layers/LayerStack.h"
#include "Core/Ini/IniParser.h"
#include "Core/Timer.h"
#include "FileWatcher/FileWatcher.h"


namespace BZ {

    class Event;
    class WindowResizedEvent;
    class Layer;
    class ImGuiLayer;

    struct FrameStats {
        TimeDuration lastFrameTime;
        uint64 frameCount;
        TimeDuration runningTime;
    };


    class Application {
    public:
        Application();
        virtual ~Application();

        void run();
        
        void onEvent(Event &ev);

        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);

        Window& getWindow() { return *window; }
        GraphicsContext& getGraphicsContext() { return *graphicsContext; }
        Input& getInput() { return *input; }

        const FrameStats& getFrameStats() const { return frameStats; }
        const std::string& getAssetsPath() const { return assetsPath; }

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher& getFileWatcher() { return fileWatcher; }
#endif      
        static Application& getInstance() { return *instance; }

    private:
        bool onWindowResized(const WindowResizedEvent &e);

        std::unique_ptr<Window> window;
        std::unique_ptr<GraphicsContext> graphicsContext;
        std::unique_ptr<Input> input;

        LayerStack layerStack;
        IniParser iniParser;
        FrameStats frameStats;

        ImGuiLayer* imGuiLayer;
        std::string assetsPath;

#ifdef BZ_HOT_RELOAD_SHADERS
        FileWatcher fileWatcher;
#endif

        static Application *instance;
    };
}