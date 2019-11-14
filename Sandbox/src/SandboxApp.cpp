#include "SandboxApp.h"

#include <imgui.h>


ExampleLayer::ExampleLayer() :
    Layer("Example") {
}

void ExampleLayer::onAttach() {
}

void ExampleLayer::onGraphicsContextCreated() {
    auto &dims = application.getWindow().getDimensions();
    float w = static_cast<float>(dims.x) * 0.5f;
    float h = static_cast<float>(dims.y) * 0.5f;
    cameraController = BZ::OrthographicCameraController(-w, w, -h, h);
    cameraController.getCamera().setPosition({w, h, 0.0f});
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    cameraController.onUpdate(frameStats);

    BZ::Renderer2D::beginScene(cameraController.getCamera());

    /*for(int i = 0; i < 10; ++i) {
        auto pos = glm::vec2(glm::sin(1.0f * frameStats.runningTime.asSeconds() + (float)i * 0.2f), 0.1f * (float)i);
        pos.y = (i-5) / 10.0f;
        BZ::Renderer2D::drawQuad(pos, { 0.2f, 0.2f }, { 1.0f, 1.0f, 1.0f });
    }*/

    BZ::Renderer2D::drawQuad(pos, { 200.0f, 200.0f }, rot, { 1.0f, 1.0f, 1.0f });
    BZ::Renderer2D::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void ExampleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    auto &dims = application.getWindow().getDimensions();

    ImGui::Begin("Test");
    ImGui::SliderFloat("pos x", &pos.x, 0.0f, static_cast<float>(dims.x));
    ImGui::SliderFloat("pos y", &pos.y, 0.0f, static_cast<float>(dims.y));
    ImGui::SliderFloat("rot", &rot, 0.0f, 360.0f);
    ImGui::End();
}

BZ::Application* BZ::createApplication() {
    return new Sandbox();
}

#include <EntryPoint.h>