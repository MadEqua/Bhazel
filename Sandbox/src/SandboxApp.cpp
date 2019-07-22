#include "SandboxApp.h"

#include <glm/gtc/matrix_transform.hpp>


ExampleLayer::ExampleLayer() : Layer("Example"), camera(-1.6f, 1.6f, -0.9f, 0.9f), cameraPos(0.0f), cameraRot(0.0f) {

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };
    vertexBuffer = std::shared_ptr<BZ::VertexBuffer>(BZ::VertexBuffer::create(vertices, sizeof(vertices)));
    vertexBuffer->bind();

    BZ::BufferLayout layout = {
        {BZ::ShaderDataType::Vec3, "position"},
        {BZ::ShaderDataType::Vec3, "color"}
    };
    vertexBuffer->setLayout(layout);

    uint32 indices[] = {0, 1, 2};
    indexBuffer = std::shared_ptr<BZ::IndexBuffer>(BZ::IndexBuffer::create(indices, 3));
    indexBuffer->bind();

    vertexArray = std::shared_ptr<BZ::VertexArray>(BZ::VertexArray::create());
    vertexArray->addVertexBuffer(vertexBuffer);
    vertexArray->setIndexBuffer(indexBuffer);

    const char * v = R"(
            #version 430 core
            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec3 col;
            out vec3 vCol;

            uniform mat4 viewProjectionMatrix;
            uniform mat4 modelMatrix;

            void main() {
                vCol = col;
                gl_Position = viewProjectionMatrix * modelMatrix * vec4(pos, 1.0);
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

    shader = std::make_shared<BZ::Shader>(v, f);
}

void ExampleLayer::onUpdate(BZ::Timestep timestep) {
    const float CAMERA_MOVE_SPEED = 3.0f * timestep;
    const float CAMERA_ROT_SPEED = 180.0f * timestep;
    const float TRI_MOVE_SPEED = 3.0f * timestep;
    
    if(BZ::Input::isKeyPressed(BZ_KEY_A)) cameraPos.x -= CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_D)) cameraPos.x += CAMERA_MOVE_SPEED;
    
    if(BZ::Input::isKeyPressed(BZ_KEY_W)) cameraPos.y += CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_S)) cameraPos.y -= CAMERA_MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_Q)) cameraRot -= CAMERA_ROT_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_E)) cameraRot += CAMERA_ROT_SPEED;

    camera.setPosition(cameraPos);
    camera.setRotation(cameraRot);

    BZ::RenderCommand::setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
    BZ::RenderCommand::clear();

    BZ::Renderer::beginScene(camera);

    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {
            glm::vec3 pos(i * 0.11f, j * 0.11f, 0);
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos) * scaleMat;
            BZ::Renderer::submit(shader, vertexArray, modelMatrix);
        }
    }
    BZ::Renderer::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    BZ::EventDispatcher dispatcher(event);
    dispatcher.dispatch<BZ::KeyPressedEvent>(BZ_BIND_EVENT_FN(ExampleLayer::onKeyPressedEvent));
}

void ExampleLayer::onImGuiRender() {
}

bool ExampleLayer::onKeyPressedEvent(BZ::KeyPressedEvent &event) {
    BZ_LOG_TRACE("KeyPressed: {0}", event);
    return false;
}



BZ::Application* BZ::createApplication() {
    return new Sandbox();
}