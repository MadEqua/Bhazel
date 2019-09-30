#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "Bhazel/Window.h"
#include "Bhazel/LayerStack.h"
#include "Bhazel/Core/Ini/IniParser.h"
#include "Bhazel/Core/Timer.h"


namespace BZ {

    class Event;
    class WindowCloseEvent;
    class WindowResizeEvent;
    class Layer;
    class ImGuiLayer;

    struct FrameStats {
        TimeDuration lastFrameTime;
        uint64 frameCount;
        TimeDuration runningTime;
    };


    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void run();
        
        void onEvent(Event &ev);

        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);

        Window& getWindow() { return *window; }
        GraphicsContext& getGraphicsContext() { return *graphicsContext; }

        const FrameStats &getFrameStats() const { return frameStats; }
        const std::string& getAssetsPath() const { return assetsPath; }
        
        static Application& getInstance() { return *instance; }

    private:
        std::unique_ptr<Window> window;
        std::unique_ptr<GraphicsContext> graphicsContext;

        bool onWindowClose(WindowCloseEvent &e);
        bool onWindowResize(WindowResizeEvent &e);

        ImGuiLayer* imGuiLayer;
        bool running = true;
        bool minimized = false;

        LayerStack layerStack;
        IniParser iniParser;

        FrameStats frameStats;

        std::string assetsPath;

        static Application *instance;
    };

    //To be defined in Bhazel client applications. It should return an object inherited from Application.
    Application* createApplication();
}