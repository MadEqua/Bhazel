#pragma once

#include "bzpch.h"

#include "Bhazel/Core.h"
#include "Bhazel/Window.h"
#include "Bhazel/LayerStack.h"

#include "Bhazel/ImGui/ImGuiLayer.h"

//TODO: temporary for testing
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/Buffer.h"
#include "Bhazel/Renderer/VertexArray.h"
#include "Bhazel/Renderer/OrtographicCamera.h"
#include <memory>

namespace BZ {

    class Event;
    class WindowCloseEvent;
    class Layer;

    class BZ_API Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void run();
        
        void onEvent(Event &ev);

        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);

        inline Window& getWindow() { return *window; }
        inline static Application& getInstance() { return *instance; }
    private:
        bool onWindowClose(WindowCloseEvent &e);

        std::unique_ptr<Window> window;
        ImGuiLayer* imGuiLayer;
        bool running = true;

        LayerStack layerStack;

        //TODO: temporaries for testing
        std::shared_ptr<VertexArray> vertexArray;
        std::shared_ptr<Shader> shader;
        std::shared_ptr<VertexBuffer> vertexBuffer;
        std::shared_ptr<IndexBuffer> indexBuffer;

        OrtographicCamera camera;

        static Application *instance;
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}