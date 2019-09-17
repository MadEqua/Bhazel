#include "SandboxApp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ImGui/imgui.h>

#include <glm/gtc/random.hpp>


ExampleLayer::ExampleLayer() :
    Layer("Example") {
}

void ExampleLayer::onAttach() {
    cameraPos = {0.0f, 0.0f};
    pos = {};
    cameraRot = 0;
    disp = {0.3f, 0.3f, 0.0f};
}

void ExampleLayer::onGraphicsContextCreated() {
    camera = BZ::MakeRef<BZ::OrtographicCamera>(-16.0f / 9.0f, 16.0f / 9.0f, -1.0f, 1.0f, 0.0f, 1.0f);

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


    //Compute shader test stuff
    std::vector<Particle> particles;
    particles.resize(PARTICLE_COUNT);

    for(int i = 0; i < PARTICLE_COUNT; ++i) {
        Particle &particle = particles[i];
        particle.pos = glm::vec4(0);// glm::linearRand(glm::vec2(-1, -1), glm::vec2(1, 1));
        particle.col = glm::vec4(glm::linearRand(0.1f, 1.0f), glm::linearRand(0.1f, 1.0f), glm::linearRand(0.1f, 1.0f), 1);
    }

    BZ::BufferLayout particleLayout = {
        {BZ::DataType::Vec4, "POSITION"},
        {BZ::DataType::Vec4, "COLOR"}
    };

    particlesBuffer = BZ::Buffer::createVertexBuffer(particles.data(), particles.size() * sizeof(Particle), particleLayout);

    auto computeShader = shaderLibrary.load(BZ::Renderer::api == BZ::Renderer::API::OpenGL ? "assets/shaders/Compute.glsl" : "assets/shaders/Compute.hlsl");
    auto particleShader = shaderLibrary.load(BZ::Renderer::api == BZ::Renderer::API::OpenGL ? "assets/shaders/Particle.glsl" : "assets/shaders/Particle.hlsl");

    particlesInputDescription = BZ::InputDescription::create();
    particlesInputDescription->addVertexBuffer(particlesBuffer, particleShader);

    BZ::RenderCommand::setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
}

void ExampleLayer::onUpdate(BZ::TimeDuration deltaTime) {
    const float CAMERA_MOVE_SPEED = 2.0f * deltaTime.asSeconds();
    const float CAMERA_ROT_SPEED = 180.0f * deltaTime.asSeconds();
    const float MOVE_SPEED = 3.0f * deltaTime.asSeconds();

    if(BZ::Input::isKeyPressed(BZ_KEY_A)) cameraPos.x -= CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_D)) cameraPos.x += CAMERA_MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_W)) cameraPos.y += CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_S)) cameraPos.y -= CAMERA_MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_Q)) cameraRot -= CAMERA_ROT_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_E)) cameraRot += CAMERA_ROT_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_LEFT)) pos.x -= MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_RIGHT)) pos.x += MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_UP)) pos.y += MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_DOWN)) pos.y -= MOVE_SPEED;

    camera->setPosition(cameraPos);
    camera->setRotation(cameraRot);

    BZ::RenderCommand::clearColorAndDepthStencilBuffers();

    BZ::Renderer::submitCompute(shaderLibrary.get("Compute"), PARTICLE_COUNT / WORK_GROUP_SIZE, 1, 1, {particlesBuffer});


    BZ::Renderer::beginScene(*camera);

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
    auto particleShader = shaderLibrary.get("Particle");

    glm::mat4 modelMatrix(1.0);
    modelMatrix = glm::translate(modelMatrix, pos);
    BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    modelMatrix = glm::translate(modelMatrix, pos + disp);
    BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    BZ::Renderer::submit(particleShader, particlesInputDescription, glm::mat4(1), BZ::Renderer::RenderMode::Points);

    BZ::Renderer::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    BZ::EventDispatcher dispatcher(event);
    dispatcher.dispatch<BZ::WindowResizeEvent>(BZ_BIND_EVENT_FN(ExampleLayer::onWindowResizeEvent));
}

void ExampleLayer::onImGuiRender(BZ::TimeDuration deltaTime) {
    ImGui::Begin("Test");
    ImGui::SliderFloat3("disp", &disp[0], -1, 1);
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