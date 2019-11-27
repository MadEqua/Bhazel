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
    tex2 = BZ::Texture2D::create("textures/particle.png", BZ::TextureFormat::R8G8B8A8_sRGB);

    for(uint32 i = 0; i < OBJECT_COUNT; i++) {
        objects[i].pos = glm::linearRand({ 0.0f, 0.0f }, dims);
        objects[i].vel = glm::linearRand(glm::vec2(-300.0f, -300.0f), { 300.0f, 300.0f });
        objects[i].dims = glm::linearRand(glm::vec2(10.0f, 10.0f), { 150.0f, 150.0f });
        objects[i].tint = {1,1,1};// glm::linearRand(glm::vec3(0.0f), { 1.0f, 1.0f, 1.0f });
        objects[i].rot = glm::linearRand(0.0f, 359.0f);
        objects[i].texId = glm::linearRand(0, 1);
    }

    /*const float DIM = 200.0f;
    objects[0].pos = { 50.0f, 300.0f };
    objects[0].vel = { 0.0f, 0.0f };
    objects[0].dims = { DIM, DIM };
    objects[0].tint = { 1.0f, 0.0f, 0.0f };
    objects[0].rot = 0.0f;
    objects[0].texId = 0;

    objects[1].pos = { 50.0f+DIM, 300.0f };
    objects[1].vel = { 0.0f, 0.0f };
    objects[1].dims = { DIM, DIM };
    objects[1].tint = { 0.0f, 1.0f, 0.0f };
    objects[1].rot = 0.0f;
    objects[1].texId = 1;

    objects[2].pos = { 50.0f+2*DIM, 300.0f };
    objects[2].vel = { 0.0f, 0.0f };
    objects[2].dims = { DIM, DIM };
    objects[2].tint = { 0.0f, 0.0f, 1.0f };
    objects[2].rot = 0.0f;
    objects[2].texId = 0;

    objects[3].pos = { 50.0f+3 * DIM, 300.0f };
    objects[3].vel = { 0.0f, 0.0f };
    objects[3].dims = { DIM, DIM };
    objects[3].tint = { 1.0f, 1.0f, 0.0f };
    objects[3].rot = 0.0f;
    objects[3].texId = 1;

    objects[4].pos = { 50.0f+4 * DIM, 300.0f };
    objects[4].vel = { 0.0f, 0.0f };
    objects[4].dims = { DIM, DIM };
    objects[4].tint = { 1.0f, 0.0f, 1.0f };
    objects[4].rot = 0.0f;
    objects[4].texId = 0;

    objects[5].pos = { 50.0f, 500.0f };
    objects[5].vel = { 0.0f, 0.0f };
    objects[5].dims = { DIM, DIM };
    objects[5].tint = { 1.0f, 0.0f, 0.0f };
    objects[5].rot = 0.0f;
    objects[5].texId = 0;

    objects[6].pos = { 50.0f + DIM, 500.0f };
    objects[6].vel = { 0.0f, 0.0f };
    objects[6].dims = { DIM, DIM };
    objects[6].tint = { 0.0f, 1.0f, 0.0f };
    objects[6].rot = 0.0f;
    objects[6].texId = 1;

    objects[7].pos = { 50.0f + 2 * DIM, 500.0f };
    objects[7].vel = { 0.0f, 0.0f };
    objects[7].dims = { DIM, DIM };
    objects[7].tint = { 0.0f, 0.0f, 1.0f };
    objects[7].rot = 0.0f;
    objects[7].texId = 0;

    objects[8].pos = { 50.0f + 3 * DIM, 500.0f };
    objects[8].vel = { 0.0f, 0.0f };
    objects[8].dims = { DIM, DIM };
    objects[8].tint = { 1.0f, 1.0f, 0.0f };
    objects[8].rot = 0.0f;
    objects[8].texId = 1;

    objects[9].pos = { 50.0f + 4 * DIM, 500.0f };
    objects[9].vel = { 0.0f, 0.0f };
    objects[9].dims = { DIM, DIM };
    objects[9].tint = { 1.0f, 0.0f, 1.0f };
    objects[9].rot = 0.0f;
    objects[9].texId = 0;*/
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

        BZ::Renderer2D::drawQuad(objects[i].pos, objects[i].dims, objects[i].rot, objects[i].texId ? tex1 : tex2, objects[i].tint);
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