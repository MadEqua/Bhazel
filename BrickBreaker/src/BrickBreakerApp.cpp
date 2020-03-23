#include "BrickBreakerApp.h"

#include <glm/gtc/random.hpp>


void Ball::init(const BZ::Ref<BZ::Texture2D> &ballTexture, const BZ::Ref<BZ::Texture2D> &ballParticleTexture) {
    sprite.dimensions = ballTexture->getDimensions();
    sprite.rotationDeg = 0.0f;
    sprite.texture = ballTexture;
    sprite.tintAndAlpha = BALL_TINT;
    secsToTint = 0.0f;

    setToInitialPosition();

    BZ::Particle2DRanges ranges;
    ranges.dimensionRange = { { 15.0f, 15.0f }, { 20.0f, 20.0f } };
    ranges.angularVelocityRange = { -180.0f, 180.0f };
    ranges.tintAndAlphaRange = BALL_TINT;
    particleSystem.addEmitter({ 0.0f, 0.0f }, 100, -1, ranges, ballParticleTexture);
    particleSystem.start();
}

void Ball::onUpdate(const BZ::FrameStats &frameStats, BrickMap &brickMap, Paddle &paddle) {
    const auto WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensionsFloat();

    sprite.position += velocity * frameStats.lastFrameTime.asSeconds();

    if (sprite.position.x < 0) {
        sprite.position.x = 0;
        velocity.x = -velocity.x;
    }
    else if (sprite.position.x > WINDOW_DIMS.x) {
        sprite.position.x = WINDOW_DIMS.x;
        velocity.x = -velocity.x;
    }

    if (sprite.position.y < PADDLE_DIMS.y) {
        setToInitialPosition();
    }
    else if (sprite.position.y > WINDOW_DIMS.y) {
        sprite.position.y = WINDOW_DIMS.y;
        velocity.y = -velocity.y;
    }

    boundingSphere = BZ::BoundingSphere(glm::vec3(sprite.position, 0.1f), BALL_RADIUS);

    for (Brick& brick : brickMap.bricks) {
        if (brick.isCollidable) {
            auto intResult = BZ::CollisionUtils::intersects(brick.aabb, boundingSphere);
            if (intResult.intersects) {
                sprite.position.x += intResult.penetration.x;
                sprite.position.y += intResult.penetration.y;
                //velocity = glm::normalize(intResult.penetration) * BALL_SPEED;
                velocity = glm::reflect(velocity, glm::normalize(glm::vec2(intResult.penetration)));
                brick.isCollidable = false;
                brick.secsToFade = BRICK_FADE_SECONDS;

                colorToTint = brick.sprite.tintAndAlpha;
                secsToTint = BALL_TINT_SECONDS;

                brickMap.startParticleSystem(brick);
            }
        }
    }

    auto intResult = BZ::CollisionUtils::intersects(paddle.aabb, boundingSphere);
    if (intResult.intersects) {
        sprite.position.x += intResult.penetration.x;
        sprite.position.y += intResult.penetration.y;

        //[left, right] -> [-1, 1]
        //float positionInPaddle = (((ball.sprite.position.x - paddle.sprite.position.x) / paddle.sprite.dimensions.x) * 2.0f);
        //const glm::vec2 MAX_DISPLACEMENT = { 1.0f, 0.0f };
        //ball.velocity = glm::normalize((glm::normalize(glm::vec2(intResult.penetration.x, intResult.penetration.y)) + (MAX_DISPLACEMENT * positionInPaddle))) * BALL_SPEED;
        velocity = glm::reflect(velocity, glm::normalize(glm::vec2(intResult.penetration)));
    }

    if (secsToTint > 0.0f) {
        sprite.tintAndAlpha = glm::mix(BALL_TINT, colorToTint, secsToTint / BALL_TINT_SECONDS);
        secsToTint -= frameStats.lastFrameTime.asSeconds();
    }
    BZ::Renderer2D::drawSprite(sprite);

    for (auto &emitter : particleSystem.getEmitters()) {
        emitter.ranges.tintAndAlphaRange = sprite.tintAndAlpha;
    }

    particleSystem.setPosition(sprite.position);
    particleSystem.onUpdate(frameStats);
    BZ::Renderer2D::drawParticleSystem2D(particleSystem);
}

void Ball::setToInitialPosition() {
    const auto WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensionsFloat();
    const auto WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;

    sprite.position = { WINDOW_HALF_DIMS.x, PADDLE_Y + BRICK_MARGIN };
    velocity = glm::normalize(glm::vec2(glm::linearRand(-1.0f, 1.0f), 1.0f)) * BALL_SPEED;
}

void Paddle::init(const BZ::Ref<BZ::Texture2D> &texture) {
    const auto WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensionsFloat();
    const auto WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;

    sprite.position = { WINDOW_HALF_DIMS.x, PADDLE_Y };
    sprite.dimensions = texture->getDimensions();
    sprite.rotationDeg = 0.0f;
    sprite.texture = texture;
    sprite.tintAndAlpha = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Paddle::onUpdate(const BZ::FrameStats &frameStats) {
    const auto WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensionsFloat();

    auto &input = BZ::Application::getInstance().getInput();
    if (input.isKeyPressed(BZ_KEY_LEFT)) {
        sprite.position.x -= PADDLE_VELOCITY * frameStats.lastFrameTime.asSeconds();
    }
    if (input.isKeyPressed(BZ_KEY_RIGHT)) {
        sprite.position.x += PADDLE_VELOCITY * frameStats.lastFrameTime.asSeconds();
    }

    if (sprite.position.x - PADDLE_HALF_DIMS.x < 0) {
        sprite.position.x = PADDLE_HALF_DIMS.x;
    }
    else if (sprite.position.x + PADDLE_HALF_DIMS.x > WINDOW_DIMS.x) {
        sprite.position.x = WINDOW_DIMS.x - PADDLE_HALF_DIMS.x;
    }

    aabb = BZ::AABB(glm::vec3(sprite.position, 0.1f), glm::vec3(PADDLE_DIMS, 0.1f));
    BZ::Renderer2D::drawSprite(sprite);
    //BZ::Renderer2D::drawQuad(glm::vec2(aabb.getCenter()), glm::vec2(aabb.getDimensions()), 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
}

void BrickMap::init(const BZ::Ref<BZ::Texture2D> &brickTexture, const BZ::Ref<BZ::Texture2D> &explosionTexture) {
    const auto &WINDOW_DIMS = BZ::Application::getInstance().getWindow().getDimensions();

    bool flip = true;
    for (float x = BRICK_MARGIN + BRICK_HALF_DIMS.x; x < WINDOW_DIMS.x - BRICK_HALF_DIMS.x; x += BRICK_DIMS.x + BRICK_MARGIN) {
        for (float y = WINDOW_DIMS.y - BRICK_MARGIN - BRICK_HALF_DIMS.y; y > (WINDOW_DIMS.y / 2) - BRICK_HALF_DIMS.y; y -= BRICK_DIMS.y + BRICK_MARGIN) {
            Brick brick;
            brick.isVisible = true;
            brick.isCollidable = true;
            brick.secsToFade = 0.0f;
            brick.sprite.position = { x, y };
            brick.sprite.dimensions = brickTexture->getDimensions();
            brick.sprite.rotationDeg = 0.0f;
            brick.sprite.texture = brickTexture;
            brick.sprite.tintAndAlpha = flip ? BRICK_TINT1 : BRICK_TINT2;
            brick.aabb = BZ::AABB(glm::vec3(brick.sprite.position, 0.1f), glm::vec3(BRICK_DIMS, 0.1f));
            bricks.push_back(brick);
            flip = !flip;
        }
    }

    for (int i = 0; i < PARTICLE_SYSTEMS_COUNT; ++i) {
        BZ::ParticleSystem2D &ps = particleSystems[i];
        BZ::Particle2DRanges ranges;
        ranges.dimensionRange = { { 20.0f, 20.0f }, { 30.0f, 30.0f } };
        ranges.velocityRange = { { -300.0f, -300.0f }, { 300.0f, 300.0f } };
        ranges.angularVelocityRange = { -180.0f, 180.0f };
        ranges.tintAndAlphaRange = BRICK_HIT_TINT;
        ranges.lifeSecsRange = { 0.5f, 0.75f };
        ps.addEmitter({ 0.0f, 0.0f }, 500, 0.05f, ranges, explosionTexture);
    }

    currentParticleSystem = 0;
}

void BrickMap::onUpdate(const BZ::FrameStats &frameStats) {
    for (uint32 i = 0; i < bricks.size(); ++i) {
        Brick &brick = bricks[i];
        if (brick.isVisible) {
            if (brick.secsToFade > 0.0f) {
                //brick.sprite.tintAndAlpha = { 1.0f, 0.0f, 0.0f, brick.secsToFade / BRICK_FADE_SECONDS };
                brick.sprite.tintAndAlpha = BRICK_HIT_TINT;
                brick.sprite.tintAndAlpha.a = brick.secsToFade / BRICK_FADE_SECONDS;
                brick.secsToFade -= frameStats.lastFrameTime.asSeconds();
                if (brick.secsToFade <= 0.0f) {
                    brick.isVisible = false;
                }
            }

            BZ::Renderer2D::drawSprite(brick.sprite);
        }
    }

    for (int i = 0; i < PARTICLE_SYSTEMS_COUNT; ++i) {
        particleSystems[i].onUpdate(frameStats);
        BZ::Renderer2D::drawParticleSystem2D(particleSystems[i]);
    }
}

void BrickMap::startParticleSystem(const Brick &brick) {
    BZ::ParticleSystem2D &ps = particleSystems[currentParticleSystem];
    
    ps.setPosition(brick.sprite.position);
    ps.start();

    currentParticleSystem = (currentParticleSystem + 1) % PARTICLE_SYSTEMS_COUNT;
}


MainLayer::MainLayer() :
    Layer("MainLayer") {
}

void MainLayer::onAttach() {
}

void MainLayer::onGraphicsContextCreated() {
    const auto WINDOW_DIMS = application.getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = { WINDOW_DIMS.x * 0.5f, WINDOW_DIMS.t * 0.5f };

    camera = BZ::OrthographicCamera(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    camera.getTransform().setTranslation({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f });
    cameraController = BZ::OrthographicCameraController(camera);

    brickTexture = BZ::Texture2D::create("BrickBreaker/textures/brick.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);
    paddleTexture = BZ::Texture2D::create("BrickBreaker/textures/paddle.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);
    ballTexture = BZ::Texture2D::create("BrickBreaker/textures/ball.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);
    ballParticleTexture = BZ::Texture2D::create("BrickBreaker/textures/particle2.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);
    brickExplosionTexture = BZ::Texture2D::create("BrickBreaker/textures/particle1.png", BZ::TextureFormat::R8G8B8A8_SRGB, true);

    brickMap.init(brickTexture, brickExplosionTexture);
    paddle.init(paddleTexture);
    ball.init(ballTexture, ballParticleTexture);
}

void MainLayer::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    cameraController.onUpdate(frameStats);

    BZ::Renderer2D::beginScene(cameraController.getCamera());

    brickMap.onUpdate(frameStats);
    paddle.onUpdate(frameStats);
    ball.onUpdate(frameStats, brickMap, paddle);

    BZ::Renderer2D::endScene();
}

void MainLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void MainLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();
}

BZ::Application* createApplication() {
    return new BrickBreakerApp();
}