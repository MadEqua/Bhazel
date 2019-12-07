#include "bzpch.h"

#include "ParticleSystem2D.h"
#include "Core/Application.h"


namespace BZ {

    Particle2DRanges::Particle2DRanges() :
        positionRange(glm::vec2(0.0f)),
        dimensionRange(glm::vec2(10.0f, 10.0f), glm::vec2(15.0f, 15.0f)),
        rotationRange(0.0f, 359.0f),
        lifeSecsRange(1.0f, 5.0f),
        velocityRange(glm::vec2(-50.0f, -50.0f), glm::vec2(50.0f, 50.0f)),
        angularVelocityRange(0.0f, 0.0f),
        accelerationRange(glm::vec3(0.0f), glm::vec2(0.0f)),
        tintAndAlphaRange(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), glm::vec4(1.0f)) {
    }

    Emitter2D::Emitter2D(ParticleSystem2D &parent, const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs, Particle2DRanges &ranges, const Ref<Texture2D> &texture) :
        parent(parent),
        positionOffset(positionOffset),
        particlesPerSec(particlesPerSec),
        secsPerParticle(1.0f / particlesPerSec),
        secsUntilNextEmission(1.0f / particlesPerSec),
        totalLifeSecs(totalLifeSecs),
        secsToLive(totalLifeSecs),
        texture(texture),
        ranges(ranges) {
    }

    void Emitter2D::start() {
        secsUntilNextEmission = 1.0f / particlesPerSec;
        secsToLive = totalLifeSecs;
        activeParticles.clear();
    }

    void Emitter2D::onUpdate(const FrameStats &frameStats) {
        //Update lifetimes
        for (auto &particle : activeParticles) {
            particle.timeToLiveSecs -= frameStats.lastFrameTime.asSeconds();
        }

        auto it = std::remove_if(activeParticles.begin(), activeParticles.end(), [](const Particle2D &particle) {
            return particle.timeToLiveSecs <= 0.0f;
        });
        activeParticles.erase(it, activeParticles.end());
        
        //Update current active particles
        for (auto &particle : activeParticles) {
            particle.velocity += particle.acceleration * frameStats.lastFrameTime.asSeconds();
            particle.position += particle.velocity * frameStats.lastFrameTime.asSeconds();
            particle.rotationDeg += particle.angularVelocity * frameStats.lastFrameTime.asSeconds();
            particle.timeToLiveSecs -= frameStats.lastFrameTime.asSeconds();
            particle.tintAndAlpha.a = particle.originalAlpha * (particle.timeToLiveSecs / particle.totalLifeSecs);
        }

        //If not immortal (negative totalLife) and not dead, decrement life
        if (totalLifeSecs >= 0.0f && secsToLive >= 0.0f) {
            secsToLive -= frameStats.lastFrameTime.asSeconds();
        }

        //If immortal or still alive then emit
        if (totalLifeSecs < 0.0f || secsToLive >= 0.0f) {
            secsUntilNextEmission -= frameStats.lastFrameTime.asSeconds();
            if (secsUntilNextEmission < 0.0f) {
                uint32 countToEmit = static_cast<uint32>(-secsUntilNextEmission / secsPerParticle);
                if (countToEmit > 0) {
                    //BZ_LOG_DEBUG("emitting {}", countToEmit);
                    for (uint32 i = 0; i < countToEmit; ++i) {
                        emitParticle();
                    }

                    //TODO: find out the correct operation
                    //secsUntilNextEmission += secsPerParticle;
                    secsUntilNextEmission = secsPerParticle;
                }
            }
        }
    }

    void Emitter2D::emitParticle() {
        Particle2D particle;
        particle.position = parent.getPosition() + positionOffset + ranges.positionRange.getValue();
        particle.dimensions = ranges.dimensionRange.getValue();
        particle.rotationDeg = ranges.rotationRange.getValue();
        particle.tintAndAlpha = ranges.tintAndAlphaRange.getValue();
        particle.originalAlpha = particle.tintAndAlpha.a;
        particle.totalLifeSecs = ranges.lifeSecsRange.getValue();
        particle.timeToLiveSecs = particle.totalLifeSecs;
        particle.velocity = ranges.velocityRange.getValue();
        particle.angularVelocity = ranges.angularVelocityRange.getValue();
        particle.acceleration = ranges.accelerationRange.getValue();
        activeParticles.push_back(particle);
    }

    ParticleSystem2D::ParticleSystem2D() : 
        position({0.0f}) {
    }

    ParticleSystem2D::ParticleSystem2D(const glm::vec2 & position) :
        position(position) {
    }

    void ParticleSystem2D::addEmitter(const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs, Particle2DRanges &ranges, const Ref<Texture2D> &texture) {
        emitters.emplace_back(*this, positionOffset, particlesPerSec, totalLifeSecs, ranges, texture);
    }

    void ParticleSystem2D::start() {
        for (auto &emitter : emitters) {
            emitter.start();
        }
        isStarted = true;
    }

    void ParticleSystem2D::onUpdate(const FrameStats &frameStats) {
        if (isStarted) {
            for (auto &emitter : emitters) {
                emitter.onUpdate(frameStats);
            }
        }
    }
}