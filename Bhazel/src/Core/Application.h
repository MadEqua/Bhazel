#pragma once

#include "Graphics/GraphicsContext.h"
#include "Window.h"
#include "Core/Input.h"
#include "Layers/LayerStack.h"
#include "Core/Ini/IniParser.h"
#include "Core/Timer.h"


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
        virtual ~Application() = default;

        void run();
        
        void onEvent(Event &ev);

        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);

        Window& getWindow() { return *window; }
        GraphicsContext& getGraphicsContext() { return *graphicsContext; }
        Input& getInput() { return *input; }

        const FrameStats &getFrameStats() const { return frameStats; }
        const std::string& getAssetsPath() const { return assetsPath; }
        
        static Application& getInstance() { return *instance; }

    private:
        bool onWindowResized(WindowResizedEvent &e);

        std::unique_ptr<Window> window;
        std::unique_ptr<GraphicsContext> graphicsContext;
        std::unique_ptr<Input> input;

        LayerStack layerStack;
        IniParser iniParser;
        FrameStats frameStats;

        ImGuiLayer* imGuiLayer;
        std::string assetsPath;

        static Application *instance;
    };

    //To be defined in Bhazel client applications. It should return an object inherited from Application.
    Application* createApplication();
}