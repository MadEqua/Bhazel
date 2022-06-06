#pragma once

#include <glm/gtc/random.hpp>


namespace BZ {

class ParticleSystem2D;
struct FrameTiming;
class Scene;
class Texture2D;


template<typename T>
struct Range {
    T min;
    T max;

    Range(const T &min, const T &max) : min(min), max(max) {}
    Range(const T &v) : min(v), max(v) {}

    void operator=(const T &v) {
        min = v;
        max = v;
    }

    T getValue() { return glm::linearRand(min, max); }
};

struct Particle2DRanges {
    Range<glm::vec2> positionRange = { {} };
    Range<glm::vec2> dimensionRange = { { 10.0f, 10.0f }, { 15.0f, 15.0f } };
    Range<float> rotationRange = { 0.0f, 359.0f };
    Range<float> lifeSecsRange = { 1.0f, 5.0f };
    Range<glm::vec2> velocityRange = { { -50.0f, -50.0f }, { 50.0f, 50.0f } };
    Range<float> angularVelocityRange = { 0.0f, 0.0f };
    Range<glm::vec2> accelerationRange = { {} };
    Range<glm::vec4> tintAndAlphaRange = { { 0.2f, 0.2f, 0.2f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
};

struct Particle2D {
    glm::vec2 position;
    glm::vec2 dimensions;
    float rotationDeg;
    glm::vec4 tintAndAlpha;
    float originalAlpha;

    float timeToLiveSecs;
    float totalLifeSecs;
    glm::vec2 velocity;
    float angularVelocity;
    glm::vec2 acceleration;
};


/*-------------------------------------------------------------------------------------------*/
class Emitter2D {
  public:
    Emitter2D(const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs, Particle2DRanges &ranges,
              const Ref<Texture2D> &texture);

    void start();

    uint32 particlesPerSec;
    float totalLifeSecs;

    Particle2DRanges ranges;
    Ref<Texture2D> texture;

    // Relative to parent ParticleSystem.
    glm::vec2 positionOffset;
    float secsToLive;
    float secsPerParticle;
    float secsUntilNextEmission;

    std::vector<Particle2D> activeParticles;
    // std::vector<Particle2D> inactiveParticles;
};


/*-------------------------------------------------------------------------------------------*/
/*
 * Works on World coordinates.
 */
class ParticleSystem2D {
  public:
    ParticleSystem2D();
    explicit ParticleSystem2D(const glm::vec2 &position);

    void start();

    std::vector<Emitter2D> emitters;
    glm::vec2 position;
    bool isStarted = false;
};


/*-------------------------------------------------------------------------------------------*/
class ParticleSystem2DSystem {
  public:
    static void update(const FrameTiming &frameTiming, Scene &scene);

  private:
    static void emitParticle(const ParticleSystem2D &system, Emitter2D &emitter);
};
}
