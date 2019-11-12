#include "SandboxApp.h"

#include <imgui.h>


ExampleLayer::ExampleLayer() :
    Layer("Example") {
}

void ExampleLayer::onAttach() {
}

void ExampleLayer::onGraphicsContextCreated() {
    cameraController = BZ::MakeRef<BZ::OrthographicCameraController>(1280.0f / 800.0f);
    cameraController->getCamera().setPosition({0.0f, 0.0f, 0.0f});
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    cameraController->onUpdate(frameStats);

    BZ::Renderer2D::beginScene(cameraController->getCamera());

    /*for(int i = 0; i < 10; ++i) {
        auto pos = glm::vec2(glm::sin(1.0f * frameStats.runningTime.asSeconds() + (float)i * 0.2f), 0.1f * (float)i);
        pos.y = (i-5) / 10.0f;
        BZ::Renderer2D::drawQuad(pos, { 0.2f, 0.2f }, { 1.0f, 1.0f, 1.0f });
    }*/

    BZ::Renderer2D::drawQuad(pos, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f });
    BZ::Renderer2D::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    cameraController->onEvent(event);
}

void ExampleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    ImGui::Begin("Test");
    ImGui::SliderFloat2("pos", &pos.x, -1, 1);
    ImGui::End();
}

BZ::Application* BZ::createApplication() {
    return new Sandbox();
}

#include <EntryPoint.h>