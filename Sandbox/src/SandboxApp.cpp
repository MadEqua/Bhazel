#include "SandboxApp.h"

#include <imgui.h>
#include <glm/gtc/random.hpp>


ExampleLayer::ExampleLayer() :
    Layer("Example") {
}

void ExampleLayer::onAttach() {
}

void ExampleLayer::onGraphicsContextCreated() {
    auto &dims = application.getWindow().getDimensions();
    float halfW = static_cast<float>(dims.x) * 0.5f;
    float halfH = static_cast<float>(dims.y) * 0.5f;
    cameraController = BZ::OrthographicCameraController(-halfW, halfW, -halfH, halfH);
    cameraController.getCamera().setPosition({halfW, halfH, 0.0f});

    tex1 = BZ::Texture2D::create("textures/test.jpg", BZ::TextureFormat::R8G8B8A8_sRGB);
    tex2 = BZ::Texture2D::create("textures/alphaTest.png", BZ::TextureFormat::R8G8B8A8_sRGB);

    for(uint32 i = 0; i < OBJECT_COUNT; i++) {
        objects[i].pos = glm::linearRand({ 0.0f, 0.0f }, dims);
        objects[i].vel = glm::linearRand(glm::vec2(-300.0f, -300.0f), { 300.0f, 300.0f });
        objects[i].dims = glm::linearRand(glm::vec2(10.0f, 10.0f), { 150.0f, 150.0f });
        objects[i].rot = glm::linearRand(0.0f, 359.0f);
        objects[i].texId = glm::linearRand(0, 1);
    }
}

void ExampleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    cameraController.onUpdate(frameStats);

    auto& dims = application.getWindow().getDimensions();

    BZ::Renderer2D::beginScene(cameraController.getCamera());

    for (uint32 i = 0; i < OBJECT_COUNT; i++) {
        objects[i].pos += objects[i].vel * frameStats.lastFrameTime.asSeconds();
        if (objects[i].pos.x >= dims.x) {
            objects[i].pos.x = static_cast<float>(dims.x);
            objects[i].vel.x = -objects[i].vel.x;
        }
        else if (objects[i].pos.x <= 0) {
            objects[i].pos.x = 0.0f;
            objects[i].vel.x = -objects[i].vel.x;
        }
        if (objects[i].pos.y >= dims.y) {
            objects[i].pos.y = static_cast<float>(dims.y);
            objects[i].vel.y = -objects[i].vel.y;
        }
        else if (objects[i].pos.y <= 0) {
            objects[i].pos.y = 0.0f;
            objects[i].vel.y = -objects[i].vel.y;
        }

        BZ::Renderer2D::drawQuad(objects[i].pos, objects[i].dims, objects[i].rot, objects[i].texId ? tex1 : tex2, { 1.0f, 1.0f, 1.0f });
    }

    /*for (int i = 0; i < 512; ++i) {
        auto pos = glm::vec2(glm::sin(1.0f * frameStats.runningTime.asSeconds() + (float)i * 0.2f), 0.1f * (float)i) * 0.5f + 0.5f;
        pos.y = i / 10.0f;
        pos.x *= dims.x;
        pos.y *= dims.y;
        BZ::Renderer2D::drawQuad(pos, { 100.0f, 100.0f }, 0.0f, tex2, { 1.0f, 1.0f, 1.0f });
    }

    BZ::Renderer2D::drawQuad(pos, { 200.0f, 200.0f }, rot, tex1, { 1.0f, 1.0f, 1.0f });
    BZ::Renderer2D::drawQuad({ 600.0f, 400.0f }, { 200.0f, 200.0f }, 0.0f, tex2, { 1.0f, 1.0f, 1.0f });
    BZ::Renderer2D::drawQuad({ 900.0f, 500.0f }, { 100.0f, 200.0f }, 0.0f, { 0.0f, 1.0f, 0.0f });*/

    BZ::Renderer2D::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void ExampleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

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