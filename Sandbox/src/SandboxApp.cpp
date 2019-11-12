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
        {BZ::DataType::Float32, BZ::DataElements::Vec2, "TEXCOORD"},
        {BZ::DataType::Float32, BZ::DataElements::Vec3, "COLOR"},
    };
    float vertices[] = {
        -0.5f, -0.5f,
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.5f, -0.5f,
        1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.5f, 0.5f,
        1.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f,
        0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };
    uint16 indices[] = { 0, 1, 2, 2, 3, 0 };

    vertexBuffer = BZ::Buffer::create(BZ::BufferType::Vertex, sizeof(vertices), BZ::MemoryType::GpuOnly, dataLayout);
    vertexBuffer->setData(vertices, sizeof(vertices), 0);
    indexBuffer = BZ::Buffer::create(BZ::BufferType::Index, sizeof(indices), BZ::MemoryType::GpuOnly);
    indexBuffer->setData(indices, sizeof(indices), 0);

    texture = BZ::Texture2D::create("textures/test.jpg", BZ::TextureFormat::R8G8B8A8_sRGB);
    textureView = BZ::TextureView::create(texture);

    BZ::Sampler::Builder samplerBuilder;
    sampler = BZ::Sampler::create(samplerBuilder);

    BZ::DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
    descriptorSetLayoutBuilder.addDescriptorDesc(BZ::DescriptorType::CombinedTextureSampler, BZ::flagsToMask(BZ::ShaderStageFlags::Fragment), 1);
    BZ::Ref<BZ::DescriptorSetLayout> descriptorSetLayout = descriptorSetLayoutBuilder.build();

    descriptorSet = BZ::DescriptorSet::create(descriptorSetLayout);
    descriptorSet->setCombinedTextureSampler(textureView, sampler, 0);

    auto &windowDims = BZ::Application::getInstance().getWindow().getDimensions();

    pipelineStateData.dataLayout = dataLayout;
    pipelineStateData.primitiveTopology = BZ::PrimitiveTopology::Triangles;
    pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(windowDims.x), static_cast<float>(windowDims.y) } };
    pipelineStateData.scissorRects = { { 0u, 0u, static_cast<uint32>(windowDims.x), static_cast<uint32>(windowDims.y) } };
    pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
    pipelineStateData.blendingState.attachmentBlendingStates = { {} };
    pipelineState = BZ::PipelineState::create(pipelineStateData);
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    //cameraController->onUpdate(frameStats);

    auto commandBufferId = BZ::Graphics::beginCommandBuffer();

    BZ::ClearValues clearColor;
    clearColor.floating = {0.1f, 0.1, 0.1f, 1.0f};
    //BZ::Graphics::clearColorAttachments(commandBufferId, clearColor);
    
    BZ::Graphics::beginScene(commandBufferId, glm::mat4(1.0f), glm::mat4(1.0f));

    BZ::Graphics::bindBuffer(commandBufferId, vertexBuffer, 0);
    BZ::Graphics::bindBuffer(commandBufferId, indexBuffer, 0);
    BZ::Graphics::bindPipelineState(commandBufferId, pipelineState);
    BZ::Graphics::bindDescriptorSet(commandBufferId, descriptorSet, pipelineState, APP_FIRST_DESCRIPTOR_SET_IDX, nullptr, 0);
    
    for(int i = 0; i < 3; ++i) {
        auto model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)), glm::vec3(glm::sin(1.0f * frameStats.runningTime.asSeconds() + (float)i*0.2f), 0.1f * (float)i, 0.0f));
        BZ::Graphics::beginObject(commandBufferId, model);
        BZ::Graphics::drawIndexed(commandBufferId, 6, 1, 0, 0, 0);
    }

    BZ::Graphics::endCommandBuffer(commandBufferId);
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

#include <EntryPoint.h>