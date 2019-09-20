#include "SandboxApp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ImGui/imgui.h>

#include <glm/gtc/random.hpp>


ExampleLayer::ExampleLayer() :
    Layer("Example"), 
    particleSystem(PARTICLE_COUNT),
    cameraController(1280.0f / 800.0f) {
}

void ExampleLayer::onAttach() {
}

void ExampleLayer::onGraphicsContextCreated() {
    auto shader = shaderLibrary.load(BZ::Renderer::api == BZ::Renderer::API::OpenGL ? "assets/shaders/Texture.glsl" : "assets/shaders/Texture.hlsl");
    texture = BZ::Texture2D::create("assets/textures/test.jpg");

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f,

        0.5f, -0.5f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f,

        0.5f, 0.5f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f,

        -0.5f, 0.5f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f,
    };

    BZ::BufferLayout layout = {
        {BZ::DataType::Vec3, "POSITION"},
        {BZ::DataType::Vec3, "COLOR"},
        {BZ::DataType::Vec2, "TEXCOORD"}
    };
    vertexBuffer = BZ::Buffer::createVertexBuffer(vertices, sizeof(vertices), layout);

    uint32 indices[] = {0, 1, 2, 0, 2, 3};
    indexBuffer = BZ::Buffer::createIndexBuffer(indices, sizeof(indices));

    inputDescription = BZ::InputDescription::create();
    inputDescription->addVertexBuffer(vertexBuffer, shader);
    inputDescription->setIndexBuffer(indexBuffer);

    particleSystem.init();

    BZ::RenderCommand::setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    cameraController.onUpdate(frameStats);

    const float MOVE_SPEED = 3.0f * frameStats.lastFrameTime.asSeconds();

    /*if(BZ::Input::isKeyPressed(BZ_KEY_LEFT)) particleSystemPosition.x -= MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_RIGHT)) particleSystemPosition.x += MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_UP)) particleSystemPosition.y += MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_DOWN)) particleSystemPosition.y -= MOVE_SPEED;*/

    //TODO: coordinate conversion
    /**if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_1)) {
        particleSystemPosition.x = BZ::Input::getMouseX();
        particleSystemPosition.y = BZ::Input::getMouseY();
    }*/

    BZ::RenderCommand::clearColorAndDepthStencilBuffers();


    BZ::Renderer::beginScene(cameraController.getCamera(), frameStats);

    texture->bindToPipeline(0);

    /*glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {
            glm::vec3 pos(i * 0.31f, j * 0.31f, 0);
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos) * scaleMat;
            BZ::Renderer::submit(shader, inputDescription, modelMatrix);
        }
    }*/

    auto textureShader = shaderLibrary.get("Texture");

    glm::mat4 modelMatrix(1.0);
    modelMatrix = glm::translate(modelMatrix, pos);
    BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    modelMatrix = glm::translate(modelMatrix, pos + disp);
    //BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    particleSystem.onUpdate();

    BZ::Renderer::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);

    BZ::EventDispatcher dispatcher(event);
    dispatcher.dispatch<BZ::WindowResizeEvent>(BZ_BIND_EVENT_FN(ExampleLayer::onWindowResizeEvent));
}

void ExampleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    ImGui::Begin("Test");
    ImGui::SliderFloat3("disp", &disp[0], -1, 1);
    ImGui::End();

    constexpr float LIMIT = 0.5f;
    constexpr float LIMIT2 = 1.5f;
    //TODO: temporary
    if(ImGui::Begin("Particles")) {
        ImGui::Text("Emitter Position");
        ImGui::SliderFloat3("##emmiterpos", &particleSystem.position[0], -LIMIT2, LIMIT2);
        ImGui::Text("Emitter Scale");
        ImGui::SliderFloat3("##emmitersc", &particleSystem.scale[0], 0.0f, LIMIT2);
        ImGui::Text("Position Rotation");
        ImGui::SliderFloat3("##emmiterrot", &particleSystem.eulerAngles[0], 0.0f, 359.0f);
        ImGui::Separator();

        ImGui::Text("Position Range");
        ImGui::SliderFloat3("Min##pos", &particleSystem.ranges.positionRange.min[0], -LIMIT, LIMIT);
        ImGui::SliderFloat3("Max##pos", &particleSystem.ranges.positionRange.max[0], -LIMIT, LIMIT);
        ImGui::Separator();
        ImGui::Text("Size Range");
        ImGui::SliderFloat3("Min##sz", &particleSystem.ranges.sizeRange.min[0], 0.0f, LIMIT * 2.0f);
        ImGui::SliderFloat3("Max##sz", &particleSystem.ranges.sizeRange.max[0], 0.0f, LIMIT * 2.0f);
        ImGui::Separator();
        ImGui::Text("Life Range");
        ImGui::SliderFloat("Min##life", &particleSystem.ranges.lifeRange.min, 0, 10);
        ImGui::SliderFloat("Max##life", &particleSystem.ranges.lifeRange.max, 0, 10);
        ImGui::Separator();
        ImGui::Text("Velocity Range");
        ImGui::SliderFloat3("Min##vel", &particleSystem.ranges.velocityRange.min[0], -LIMIT2, LIMIT2);
        ImGui::SliderFloat3("Max##vel", &particleSystem.ranges.velocityRange.max[0], -LIMIT2, LIMIT2);
        ImGui::Separator();
        ImGui::Text("Acceleration Range");
        ImGui::SliderFloat3("Min##accel", &particleSystem.ranges.accelerationRange.min[0], -LIMIT2, LIMIT2);
        ImGui::SliderFloat3("Max##accel", &particleSystem.ranges.accelerationRange.max[0], -LIMIT2, LIMIT2);
        ImGui::Separator();
        ImGui::Text("Tint Range");
        ImGui::SliderFloat3("Min##tint", &particleSystem.ranges.tintRange.min[0], 0, 1);
        ImGui::SliderFloat3("Max##tint", &particleSystem.ranges.tintRange.max[0], 0, 1);
        ImGui::Separator();
    }
    ImGui::End();
}

bool ExampleLayer::onWindowResizeEvent(BZ::WindowResizeEvent &ev) {
    BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    return false;
}



BZ::Application* BZ::createApplication() {
    return new Sandbox();
}

#include <Bhazel/EntryPoint.h>