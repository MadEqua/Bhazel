#include "SandboxApp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <glm/gtc/random.hpp>


ExampleLayer::ExampleLayer() :
    Layer("Example") {
}

void ExampleLayer::onAttach() {
}

void ExampleLayer::onGraphicsContextCreated() {
    //cameraController = BZ::MakeRef<BZ::PerspectiveCameraController>(60.0f, 1280.0f / 800.0f);
    //cameraController->getCamera().setPosition({0.0f, 0.0f, 1.5f});*/

    BZ::Shader::Builder shaderBuilder;
    shaderBuilder.setName("test");
    shaderBuilder.fromBinaryFile(BZ::ShaderStage::Vertex, "shaders/bin/vert.spv");
    shaderBuilder.fromBinaryFile(BZ::ShaderStage::Fragment, "shaders/bin/frag.spv");
    
    BZ::PipelineStateData pipelineStateData;
    pipelineStateData.shader = shaderBuilder.build();

    BZ::DataLayout dataLayout = {
        {BZ::DataType::Float32, BZ::DataElements::Vec2, "POSITION"},
        {BZ::DataType::Float32, BZ::DataElements::Vec3, "COLOR"},
    };
    float vertices[] = {
        -0.5f, -0.5f,
        1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,
        0.0f, 1.0f, 0.0f,
        0.5f, 0.5f,
        0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f,
        1.0f, 1.0f, 1.0f,
    };
    uint16 indices[] = { 0, 1, 2, 2, 3, 0 };

    vertexBuffer = BZ::Buffer::createVertexBuffer(vertices, sizeof(vertices), dataLayout);
    indexBuffer = BZ::Buffer::createIndexBuffer(indices, sizeof(indices));

    constantBuffer = BZ::Buffer::createConstantBuffer(sizeof(ConstantData));
    constantBuffer->setData(&constantData, sizeof(ConstantData));

    BZ::DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
    descriptorSetLayoutBuilder.addDescriptorDesc(BZ::DescriptorType::ConstantBuffer, BZ::flagsToMask(BZ::ShaderStageFlags::Vertex), 1);
    BZ::Ref<BZ::DescriptorSetLayout> descriptorSetLayout = descriptorSetLayoutBuilder.build();

    //TODO: We don't want the app worrying about pools
    BZ::DescriptorPool::Builder builder;
    builder.addDescriptorTypeCount(BZ::DescriptorType::ConstantBuffer, 1024);
    descriptorPool = builder.build();
    descriptorSet = descriptorPool->getDescriptorSet(descriptorSetLayout);
    descriptorSet->setConstantBuffer(constantBuffer, 0, 0, sizeof(constantData));

    auto &windowDims = BZ::Application::getInstance().getWindow().getDimensions();

    pipelineStateData.dataLayout = dataLayout;
    pipelineStateData.primitiveTopology = BZ::PrimitiveTopology::Triangles;
    pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(windowDims.x), static_cast<float>(windowDims.y)} };
    pipelineStateData.blendingState.attachmentBlendingStates = { {} };
    pipelineStateData.framebuffer = BZ::Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();
    pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };

    pipelineState = BZ::PipelineState::create(pipelineStateData);
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    //cameraController->onUpdate(frameStats);

    //const float MOVE_SPEED = 3.0f * frameStats.lastFrameTime.asSeconds();

    ////TODO: coordinate conversion
    ///**if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_1)) {
    //    particleSystemPosition.x = BZ::Input::getMouseX();
    //    particleSystemPosition.y = BZ::Input::getMouseY();
    //}*/

    //BZ::RenderCommand::clearColorAndDepthStencilBuffers();

    //BZ::Renderer::beginScene(cameraController->getCamera(), frameStats);

    //texture->bindToPipeline(0);

    //auto textureShader = shaderLibrary.get("Texture");

    //glm::mat4 modelMatrix(1.0);
    //BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    //modelMatrix = glm::rotate(modelMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
    //BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    //modelMatrix = glm::rotate(modelMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
    //BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);

    //modelMatrix = glm::rotate(modelMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
    //BZ::Renderer::submit(textureShader, inputDescription, modelMatrix);*/

    //particleSystem.onUpdate();

    //BZ::Renderer::endScene();

    constantData.model = glm::translate(glm::mat4(1.0f), glm::vec3(glm::sin(frameStats.runningTime.asSeconds()), 0.0f, 0.0f));
    constantBuffer->setData(&constantData, sizeof(ConstantData));

    auto commandBuffer = BZ::Renderer::startRecording();
    BZ::Renderer::bindVertexBuffer(commandBuffer, vertexBuffer);
    BZ::Renderer::bindIndexBuffer(commandBuffer, indexBuffer);
    BZ::Renderer::bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    BZ::Renderer::bindPipelineState(commandBuffer, pipelineState);
    BZ::Renderer::drawIndexed(commandBuffer, 6, 1, 0, 0, 0);
    BZ::Renderer::endRecording(commandBuffer);

    BZ::Renderer::submitCommandBuffer(commandBuffer);
}

void ExampleLayer::onEvent(BZ::Event &event) {
    //cameraController->onEvent(event);
}

void ExampleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    /*constexpr float LIMIT = 0.5f;
    constexpr float LIMIT2 = 1.5f;

    static BZ::Timer testTimer;

    //TODO: temporary
    /*if(ImGui::Begin("Particles")) {
        ImGui::Text("Emitter Position");
        ImGui::SliderFloat3("##emmiterpos", &particleSystem.position[0], -LIMIT2, LIMIT2);
        ImGui::Text("Emitter Scale");
        ImGui::SliderFloat3("##emmitersc", &particleSystem.scale[0], 0.0f, LIMIT2);
        ImGui::Text("Emitter Rotation");
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

        ImGui::LabelText("Timer test", "Counted time: %02f", testTimer.getCountedTime().asSeconds());
        if(ImGui::Button("start")) testTimer.start();
        if(ImGui::Button("pause")) testTimer.pause();
        if(ImGui::Button("reset")) testTimer.reset();
    }
    ImGui::End();*/
}

BZ::Application* BZ::createApplication() {
    return new Sandbox();
}

#include <Bhazel/EntryPoint.h>