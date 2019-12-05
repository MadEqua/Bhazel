#pragma once

#include <Bhazel.h>

const glm::vec2 PADDLE_DIMS = { 250.0f, 25.0f };
const glm::vec2 PADDLE_HALF_DIMS = PADDLE_DIMS * 0.5f;
const float PADDLE_VELOCITY = 600.0f;

const float BALL_RADIUS = 5.0f;
const float BALL_SPEED = 500.0f;

const float BRICK_FADE_SECONDS = 1.0f;

struct Brick {
    BZ::Sprite sprite;
    bool isVisible;
    bool isCollidable;
    BZ::AABB aabb;
    float secsToFade;
};

struct Ball {
    BZ::Sprite sprite;
    BZ::BoundingSphere boundingSphere;
    glm::vec2 velocity;
};

struct Paddle {
    BZ::Sprite sprite;
    BZ::AABB aabb;
};

struct BrickMap {
public:
    BrickMap();

    void initBlocks(const BZ::Ref<BZ::Texture2D> &texture);
    void draw(const BZ::FrameStats &frameStats);

    std::vector<Brick> bricks;
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