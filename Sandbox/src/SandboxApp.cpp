#include "SandboxApp.h"

#include <imgui.h>


ParticleLayer::ParticleLayer() :
    Layer("Particle") {
}

void ParticleLayer::onAttach() {
}

void ParticleLayer::onGraphicsContextCreated() {
    const auto &WINDOW_DIMS = application.getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = { WINDOW_DIMS.x * 0.5f, WINDOW_DIMS.t * 0.5f };
    cameraController = BZ::OrthographicCameraController(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    cameraController.getCamera().setPosition({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f });

    tex1 = BZ::Texture2D::create("Sandbox/textures/alphatest.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);
    tex2 = BZ::Texture2D::create("Sandbox/textures/particle.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);

    particleSystem.setPosition({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y});
    BZ::Particle2DRanges ranges;
    ranges.lifeSecsRange = { 3.0f, 6.0f };
    ranges.dimensionRange = { { 5.0f, 5.0f }, { 10.0f, 10.0f } };
    ranges.tintAndAlphaRange = { {0.1f, 0.1f, 0.7f, 1.0f}, { 0.2f, 0.5f, 0.9f, 1.0f } };
    ranges.velocityRange = { { -200.0f, 100.0f }, { 200.0f, 500.0f } };
    ranges.angularVelocityRange = { -100.0f, 100.0f };
    ranges.accelerationRange = { 0.0f, -100.0f };

    BZ::Particle2DRanges ranges2;
    ranges2.lifeSecsRange = { 1.0f, 3.0f };
    ranges2.dimensionRange = { { 5.0f, 5.0f }, { 11.0f, 11.0f } };
    ranges2.tintAndAlphaRange = { 1.0f, 1.0f, 1.0f, 1.0f };
    ranges2.velocityRange = { { -60.0f, -200.0f }, { 60.0f, -100.0f } };
    ranges2.angularVelocityRange = { -300.0f, 300.0f };
    ranges2.accelerationRange = { 0.0f, -100.0f };
    
    particleSystem.addEmitter({ 0.0f, 0.0f }, 3'000, -1, ranges2, tex1);
    particleSystem.addEmitter({ 0.0f, 0.0f }, 50'000, -1, ranges, tex2);

    particleSystem.start();
}

void ParticleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    auto WINDOW_DIMS = application.getWindow().getDimensions();

    cameraController.onUpdate(frameStats);

    static glm::vec2 pos = { 0.0f, WINDOW_DIMS.y * 0.5f };
    static bool right = true;

    if (right && pos.x >= WINDOW_DIMS.x) right = false;
    else if (!right && pos.x <= 0.0f) right = true;
    float mult = right ? 1.0f : -1.0f;

    pos.x += 100.0f * frameStats.lastFrameTime.asSeconds() * mult;
    pos.y = (sin(pos.x * 0.02f) * 0.5f + 0.5f) * (WINDOW_DIMS.y * 0.75f) + (WINDOW_DIMS.y * 0.125f);
    particleSystem.setPosition(pos);

    BZ::Renderer2D::beginScene(cameraController.getCamera());
    particleSystem.onUpdate(frameStats);
    BZ::Renderer2D::drawParticleSystem2D(particleSystem);
    BZ::Renderer2D::endScene();
}

void ParticleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void ParticleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();
}



Layer3D::Layer3D() :
    Layer("3D") {
}

void Layer3D::onAttach() {
}

void Layer3D::onGraphicsContextCreated() {
    cameraController = BZ::PerspectiveCameraController(50.0f, application.getWindow().getRatio());
    cameraController.getCamera().setPosition({ 0.0f, 0.0f, 5.0f });
}

void Layer3D::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    //auto WINDOW_DIMS = application.getWindow().getDimensions();

    cameraController.onUpdate(frameStats);

    BZ::Renderer::beginScene(cameraController.getCamera());
    BZ::Renderer::drawCube(BZ::Transform());
    BZ::Renderer::endScene();
}

void Layer3D::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void Layer3D::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();
}



BZ::Application* createApplication() {
    return new Sandbox();
}