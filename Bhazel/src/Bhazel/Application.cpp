#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Bhazel/Layer.h"
#include "Input.h"

#include <Glad/glad.h>

namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() {
        BZ_CORE_ASSERT(!instance, "Application already exists")
        instance = this;

        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BZ_BIND_EVENT_FN(Application::onEvent));

        imGuiLayer = new ImGuiLayer();
        pushOverlay(imGuiLayer);

        //TESTING
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::create(vertices, sizeof(vertices)));
        vertexBuffer->bind();

        BufferLayout layout = {
            {ShaderDataType::Vec3, "position"},
            {ShaderDataType::Vec3, "color"}
        };
        vertexBuffer->setLayout(layout);

        unsigned int indices[] = { 0, 1, 2 };
        indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::create(indices, 3));
        indexBuffer->bind();

        const char * v = R"(
            #version 450 core
            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec3 col;
            out vec3 vCol;
            
            void main() {
                gl_Position = vec4(pos, 1.0);
                vCol = col;
            }
        )";
        const char * f = R"(
            #version 450 core
            layout(location = 0) out vec4 col;
            in vec3 vCol;
            
            void main() {
                col = vec4(vCol, 1.0);
            }
        )";

        shader = std::make_unique<Shader>(v, f);
    }

    void Application::run() {
        while(running) {

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader->bind();
            glBindVertexArray(vertexArray);
            glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr);

            for (Layer *layer : layerStack) {
                layer->onUpdate();
            }

            imGuiLayer->begin();
            for(Layer *layer : layerStack) {
                layer->onImGuiRender();
            }
            imGuiLayer->end();

            window->onUpdate();
        }
    }

    void Application::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BZ_BIND_EVENT_FN(Application::onWindowClose));
        
        BZ_CORE_TRACE("{0}", e);

        for (auto it = layerStack.end(); it != layerStack.begin(); ) {
            (*--it)->onEvent(e);
            if (e.handled) break;
        }
    }

    void Application::pushLayer(Layer *layer) {
        layerStack.pushLayer(layer);
    }

    void Application::pushOverlay(Layer *overlay) {
        layerStack.pushOverlay(overlay);
    }

    bool Application::onWindowClose(WindowCloseEvent &e) {
        running = false;
        return true;
    }
}