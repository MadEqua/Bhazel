#pragma once

#include <glm/gtc/random.hpp>


namespace BZ {

template <typename T> struct Range {
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
    Range<glm::vec2> positionRange;
    Range<glm::vec2> dimensionRange;
    Range<float> rotationRange;
    Range<float> lifeSecsRange;
    Range<glm::vec2> velocityRange;
    Range<float> angularVelocityRange;
    Range<glm::vec2> accelerationRange;
    Range<glm::vec4> tintAndAlphaRange;

    Particle2DRanges();
};

struct Particle2D {
    // Don't use Sprite. We don't need a Texture for all Particlesm they share the Emitter texture.
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
class ParticleSystem2D;
struct FrameTiming;
class Texture2D;


class Emitter2D {
  public:
    Emitter2D(ParticleSystem2D &parent, const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs,
              Particle2DRanges &ranges, const Ref<Texture2D> &texture);

    void start();
    void onUpdate(const FrameTiming &frameTiming);

    const std::vector<Particle2D> &getActiveParticles() const { return activeParticles; }

    uint32 particlesPerSec;
    float totalLifeSecs;

    Particle2DRanges ranges;
    Ref<Texture2D> texture;

  private:
    ParticleSystem2D &parent;

    glm::vec2 positionOffset; // Relative to parent ParticleSystem
    float secsToLive;
    float secsPerParticle;
    float secsUntilNextEmission;

    std::vector<Particle2D> activeParticles;
    // std::vector<Particle2D> inactiveParticles;

    void emitParticle();
};


/*-------------------------------------------------------------------------------------------*/
/*
 * Works on World coordinates. Meant to be rendered through a Renderer2D which can take a ParticleSystem2D and perform
 * batching.
 */
class ParticleSystem2D {
  public:
    ParticleSystem2D();
    explicit ParticleSystem2D(const glm::vec2 &position);

    void addEmitter(const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs,
                    Particle2DRanges &ranges, const Ref<Texture2D> &texture);
    void start();

    void setPosition(const glm::vec2 &position) { this->position = position; }
    const glm::vec2 &getPosition() const { return position; }

    std::vector<Emitter2D> &getEmitters() { return emitters; }
    const std::vector<Emitter2D> &getEmitters() const { return emitters; }

    void onUpdate(const FrameTiming &frameTiming);

  private:
    std::vector<Emitter2D> emitters;
    glm::vec2 position;
    bool isStarted = false;
};
}
