#include "BrickBreakerApp.h"

#include <imgui.h>
#include <glm/gtc/random.hpp>


BrickMap::BrickMap() {
}

void BrickMap::initBlocks(const BZ::Ref<BZ::Texture2D> &texture) {
    const auto &WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensions();
    const int MARGIN = 5;
    const glm::ivec2 BRICK_DIMS = { 50, 25 };
    const glm::ivec2 BRICK_HALF_DIMS = BRICK_DIMS / 2;

    for (int x = MARGIN + BRICK_HALF_DIMS.x; x < WINDOW_DIMS.x - BRICK_HALF_DIMS.x; x += BRICK_DIMS.x + MARGIN) {
        for (int y = WINDOW_DIMS.y - MARGIN - BRICK_HALF_DIMS.y; y > (WINDOW_DIMS.y / 2) - BRICK_HALF_DIMS.y; y -= BRICK_DIMS.y + MARGIN) {
            Brick brick;
            brick.isVisible = true;
            brick.isCollidable = true;
            brick.secsToFade = 0.0f;
            brick.sprite.position = { static_cast<float>(x), static_cast<float>(y) };
            brick.sprite.dimensions = { BRICK_DIMS.x, BRICK_DIMS.y };
            brick.sprite.rotationDeg = 0.0f;
            brick.sprite.texture = texture;
            brick.sprite.tintAndAlpha = { 0.7f, 0.1f, 0.2f, 1.0f };
            brick.aabb = BZ::AABB(glm::vec3(brick.sprite.position, 0.1f), glm::vec3(brick.sprite.dimensions, 0.1f));
            bricks.push_back(brick);
        }
    }
}

void BrickMap::draw(const BZ::FrameStats &frameStats) {
    for (uint32 i = 0; i < bricks.size(); ++i) {
        Brick &brick = bricks[i];
        if (brick.isVisible) {
            if (brick.secsToFade > 0.0f) {
                brick.secsToFade -= frameStats.lastFrameTime.asSeconds();
                if (brick.secsToFade <= 0.0f) {
                    brick.isVisible = false;
                }
                brick.sprite.tintAndAlpha = { 1.0f, 0.0f, 0.0f, brick.secsToFade / BRICK_FADE_SECONDS };
            }

            BZ::Renderer2D::drawSprite(brick.sprite);
        }
    }

    /*for (uint32 i = 0; i < bricks.size(); ++i) {
        if (bricks[i].isVisible)
            BZ::Renderer2D::drawQuad(bricks[i].aabb.getCenter(), bricks[i].aabb.getDimensions(), 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
    }*/
}


MainLayer::MainLayer() :
    Layer("MainLayer") {
}

void MainLayer::onAttach() {
}

void MainLayer::onGraphicsContextCreated() {
    const auto WINDOW_DIMS = application.getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = { WINDOW_DIMS.x * 0.5f, WINDOW_DIMS.t * 0.5f };
    cameraController = BZ::OrthographicCameraController(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    cameraController.getCamera().setPosition({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f});

    brickTexture = BZ::Texture2D::create("BrickBreaker/textures/brick.png", BZ::TextureFormat::R8G8B8A8_sRGB);
    paddleTexture = BZ::Texture2D::create("BrickBreaker/textures/paddle.png", BZ::TextureFormat::R8G8B8A8_sRGB);
    ballTexture = BZ::Texture2D::create("BrickBreaker/textures/ball.png", BZ::TextureFormat::R8G8B8A8_sRGB);

    brickMap.initBlocks(brickTexture);

    paddle.sprite.position = { WINDOW_HALF_DIMS.x, 25.0f };
    paddle.sprite.dimensions = PADDLE_DIMS;
    paddle.sprite.rotationDeg = 0.0f;
    paddle.sprite.texture = paddleTexture;
    paddle.sprite.tintAndAlpha = { 1.0f, 1.0f, 1.0f, 1.0f };

    ball.sprite.position = { WINDOW_HALF_DIMS.x, 50.0f };
    ball.sprite.dimensions = { BALL_RADIUS * 2.0f, BALL_RADIUS * 2.0f };
    ball.sprite.rotationDeg = 0.0f;
    ball.sprite.texture = ballTexture;
    ball.sprite.tintAndAlpha = { 1.0f, 1.0f, 1.0f, 1.0f };
    ball.velocity = glm::normalize(glm::vec2(1.0f, 1.0f)) * BALL_SPEED;
}

void MainLayer::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    cameraController.onUpdate(frameStats);

    const auto WINDOW_DIMS = application.getWindow().getDimensionsFloat();

    auto &input = BZ::Application::getInstance().getInput();
    if (input.isKeyPressed(BZ_KEY_LEFT)) {
        paddle.sprite.position.x -= PADDLE_VELOCITY * frameStats.lastFrameTime.asSeconds();
    }
    if (input.isKeyPressed(BZ_KEY_RIGHT)) {
        paddle.sprite.position.x += PADDLE_VELOCITY * frameStats.lastFrameTime.asSeconds();
    }
    paddle.aabb = BZ::AABB(glm::vec3(paddle.sprite.position, 0.1f), glm::vec3(paddle.sprite.dimensions, 0.1f));


    if (paddle.sprite.position.x - PADDLE_HALF_DIMS.x < 0) {
        paddle.sprite.position.x = PADDLE_HALF_DIMS.x;
    }
    else if (paddle.sprite.position.x + PADDLE_HALF_DIMS.x > WINDOW_DIMS.x) {
        paddle.sprite.position.x = WINDOW_DIMS.x - PADDLE_HALF_DIMS.x;
    }

    ball.sprite.position += ball.velocity * frameStats.lastFrameTime.asSeconds();
    if (ball.sprite.position.x < 0) {
        ball.sprite.position.x = 0;
        ball.velocity.x = -ball.velocity.x;
    }
    else if (ball.sprite.position.x > WINDOW_DIMS.x) {
        ball.sprite.position.x = WINDOW_DIMS.x;
        ball.velocity.x = -ball.velocity.x;
    }

    if (ball.sprite.position.y < PADDLE_DIMS.y) {
        const glm::vec2 WINDOW_HALF_DIMS = { WINDOW_DIMS.x * 0.5f, WINDOW_DIMS.t * 0.5f };

        ball.sprite.position = { WINDOW_HALF_DIMS.x, 50.0f };
        ball.velocity = glm::normalize(glm::vec2(1.0f, 1.0f)) * BALL_SPEED;
    }
    else if (ball.sprite.position.y > WINDOW_DIMS.y) {
        ball.sprite.position.y = WINDOW_DIMS.y;
        ball.velocity.y = -ball.velocity.y;
    }
    ball.boundingSphere = BZ::BoundingSphere(glm::vec3(ball.sprite.position, 0.1f), BALL_RADIUS);

    for (Brick& brick : brickMap.bricks) {
        if (brick.isCollidable) {
            auto intResult = BZ::CollisionUtils::intersects(brick.aabb, ball.boundingSphere);
            if(intResult.intersects) {
                ball.sprite.position.x += intResult.penetration.x;
                ball.sprite.position.y += intResult.penetration.y;
                //ball.velocity = glm::normalize(intResult.penetration) * BALL_SPEED;
                ball.velocity = glm::reflect(ball.velocity, glm::normalize(glm::vec2(-intResult.penetration)));
                brick.isCollidable = false;
                brick.secsToFade = BRICK_FADE_SECONDS;
            }
        }
    }

    auto intResult = BZ::CollisionUtils::intersects(paddle.aabb, ball.boundingSphere);
    if (intResult.intersects) {
        ball.sprite.position.x += intResult.penetration.x;
        ball.sprite.position.y += intResult.penetration.y;

        //[left, right] -> [-1, 1]
        float positionInPaddle = (((ball.sprite.position.x - paddle.sprite.position.x) / paddle.sprite.dimensions.x) * 2.0f);
        const glm::vec2 MAX_DISPLACEMENT = { 1.0f, 0.0f };
        ball.velocity = glm::normalize((glm::normalize(glm::vec2(intResult.penetration.x, intResult.penetration.y)) + (MAX_DISPLACEMENT * positionInPaddle))) * BALL_SPEED;
    }

    BZ::Renderer2D::beginScene(cameraController.getCamera());
    brickMap.draw(frameStats);
    BZ::Renderer2D::drawSprite(ball.sprite);
    BZ::Renderer2D::drawSprite(paddle.sprite);
    BZ::Renderer2D::endScene();
}

void MainLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void MainLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();
    auto &dims = application.getWindow().getDimensions();
}

BZ::Application* BZ::createApplication() {
    return new BrickBreakerApp();
}

#include <EntryPoint.h>