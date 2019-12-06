#pragma once

#include <Bhazel.h>

const float PADDLE_Y = 25.0f;
const glm::vec2 PADDLE_DIMS = { 106.0f, 8.0f };
const glm::vec2 PADDLE_HALF_DIMS = PADDLE_DIMS * 0.5f;
const float PADDLE_VELOCITY = 600.0f;

const float BALL_RADIUS = 12.0f;
const float BALL_SPEED = 600.0f;
const glm::vec4 BALL_TINT = { 1.0f, 1.0f, 1.0f, 1.0f };
const float BALL_TINT_SECONDS = 2.0f;

const float BRICK_FADE_SECONDS = 1.0f;
const glm::vec2 BRICK_DIMS = { 50.0f, 25.0f };
const glm::vec2 BRICK_HALF_DIMS = BRICK_DIMS * 0.5f;
const glm::vec4 BRICK_TINT1 = { 0.7f, 0.1f, 0.2f, 1.0f };
const glm::vec4 BRICK_TINT2 = { 0.1f, 0.7f, 0.2f, 1.0f };
const glm::vec4 BRICK_HIT_TINT = { 0.1f, 0.1f, 0.7f, 1.0f };
const float BRICK_MARGIN = 20.0f;


struct Brick {
    BZ::Sprite sprite;
    bool isVisible;
    bool isCollidable;
    BZ::AABB aabb;
    float secsToFade;

    void update(const BZ::FrameStats &frameStats);
};

struct BrickMap {
public:
    void init(const BZ::Ref<BZ::Texture2D> &texture);
    void update(const BZ::FrameStats &frameStats);

    std::vector<Brick> bricks;
};

struct Paddle {
    BZ::Sprite sprite;
    BZ::AABB aabb;

    void init(const BZ::Ref<BZ::Texture2D> &texture);
    void update(const BZ::FrameStats &frameStats);
};

struct Ball {
    BZ::Sprite sprite;
    BZ::BoundingSphere boundingSphere;
    glm::vec2 velocity;
    float secsToTint;
    glm::vec4 colorToTint;

    void init(const BZ::Ref<BZ::Texture2D> &texture);
    void update(const BZ::FrameStats &frameStats, BrickMap &brickMap, Paddle &paddle);

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
    BZ::OrthographicCameraController cameraController;
    BrickMap brickMap;
    Ball ball;
    Paddle paddle;

    BZ::Ref<BZ::Texture2D> brickTexture;
    BZ::Ref<BZ::Texture2D> paddleTexture;
    BZ::Ref<BZ::Texture2D> ballTexture;
};


class BrickBreakerApp : public BZ::Application {
public:
    BrickBreakerApp() {
        pushLayer(new MainLayer());
    }
};