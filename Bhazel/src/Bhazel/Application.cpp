#include "bzpch.h"

#include "Application.h"

#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Bhazel/Layer.h"
#include "Input.h"

#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer.h"


namespace BZ {

    Application* Application::instance = nullptr;

    Application::Application() : 
        camera(-1.6f, 1.6f, -0.9f, 0.9f) {
        BZ_CORE_ASSERT(!instance, "Application already exists")
        instance = this;

        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BZ_BIND_EVENT_FN(Application::onEvent));

        imGuiLayer = new ImGuiLayer();
        pushOverlay(imGuiLayer);

        //TESTING
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::create(vertices, sizeof(vertices)));
        vertexBuffer->bind();

        BufferLayout layout = {
            {ShaderDataType::Vec3, "position"},
            {ShaderDataType::Vec3, "color"}
        };
        vertexBuffer->setLayout(layout);

        uint32 indices[] = { 0, 1, 2 };
        indexBuffer = std::shared_ptr<IndexBuffer>(IndexBuffer::create(indices, 3));
        indexBuffer->bind();

        vertexArray = std::shared_ptr<VertexArray>(VertexArray::create());
        vertexArray->addVertexBuffer(vertexBuffer);
        vertexArray->setIndexBuffer(indexBuffer);

        const char * v = R"(
            #version 430 core
            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec3 col;
            out vec3 vCol;

            uniform mat4 viewProjection;

            void main() {
                vCol = col;
                gl_Position = viewProjection * vec4(pos, 1.0);
            }
        )";
        const char * f = R"(
            #version 430 core
            layout(location = 0) out vec4 col;
            in vec3 vCol;
            
            void main() {
                col = vec4(vCol, 1.0);
            }
        )";

        shader = std::make_shared<Shader>(v, f);
    }

    void Application::run() {
        while(running) {

            RenderCommand::setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
            RenderCommand::clear();

            Renderer::beginScene(camera);
            Renderer::submit(shader, vertexArray);
            Renderer::endScene();

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