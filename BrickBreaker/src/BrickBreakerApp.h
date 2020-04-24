#pragma once

#include <Bhazel.h>


const float PADDLE_Y = 25.0f;
const glm::vec2 PADDLE_DIMS = { 100.0f, 10.0f };
const glm::vec2 PADDLE_HALF_DIMS = PADDLE_DIMS * 0.5f;
const float PADDLE_VELOCITY = 600.0f;

const float BALL_RADIUS = 12.0f;
const float BALL_SPEED = 600.0f;
const glm::vec4 BALL_TINT = { 1.0f, 1.0f, 1.0f, 1.0f };
const float BALL_TINT_SECONDS = 2.0f;

const float BRICK_FADE_SECONDS = 0.7f;
const glm::vec2 BRICK_DIMS = { 50.0f, 25.0f };
const glm::vec2 BRICK_HALF_DIMS = BRICK_DIMS * 0.5f;
const glm::vec4 BRICK_TINT1 = { 0.6f, 0.1f, 0.2f, 1.0f };
const glm::vec4 BRICK_TINT2 = { 0.1f, 0.6f, 0.2f, 1.0f };
const glm::vec4 BRICK_HIT_TINT = { 0.9f, 0.9f, 0.1f, 1.0f };
const float BRICK_MARGIN = 20.0f;


struct Brick {
    BZ::Sprite sprite;
    bool isVisible;
    bool isCollidable;
    BZ::AABB aabb;
    float secsToFade;
};

class BrickMap {
public:
    void init(const BZ::Ref<BZ::Texture2D> &brickTexture, const BZ::Ref<BZ::Texture2D> &explosionTexture);
    void onUpdate(const BZ::FrameStats &frameStats);

    std::vector<Brick> bricks;

    void startParticleSystem(const Brick &brick);

private:
    const static uint32 PARTICLE_SYSTEMS_COUNT = 4;
    BZ::ParticleSystem2D particleSystems[PARTICLE_SYSTEMS_COUNT];
    uint32 currentParticleSystem;
};

struct Paddle {
    BZ::Sprite sprite;
    BZ::AABB aabb;

    void init(const BZ::Ref<BZ::Texture2D> &texture);
    void onUpdate(const BZ::FrameStats &frameStats);
};

class Ball {
public:
    BZ::Sprite sprite;
    BZ::BoundingSphere boundingSphere;
    glm::vec2 velocity;
    float secsToTint;
    glm::vec4 colorToTint;

    BZ::ParticleSystem2D particleSystem;

    void init(const BZ::Ref<BZ::Texture2D> &ballTexture, const BZ::Ref<BZ::Texture2D> &ballParticleTexture);
    void onUpdate(const BZ::FrameStats &frameStats, BrickMap &brickMap, Paddle &paddle);

    void setToInitialPosition();
};


class MainLayer : public BZ::Layer {
public:
    MainLayer();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(const BZ::FrameStats &frameStats) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameStats &frameStats) override;

private:
    BZ::OrthographicCamera camera;
    BZ::CameraController2D cameraController;

    BrickMap brickMap;
    Ball ball;
    Paddle paddle;

    BZ::Ref<BZ::Texture2D> brickTexture;
    BZ::Ref<BZ::Texture2D> paddleTexture;
    BZ::Ref<BZ::Texture2D> ballTexture;
    BZ::Ref<BZ::Texture2D> ballParticleTexture;
    BZ::Ref<BZ::Texture2D> brickExplosionTexture;
};


class BrickBreakerApp : public BZ::Application {
public:
    BrickBreakerApp() {
        pushLayer(new MainLayer());
    }
};