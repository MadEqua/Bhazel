#include "bzpch.h"

#include "ParticleSystem2D.h"

#include "Core/Engine.h"
#include "Graphics/Texture.h"
#include "Scene/Scene.h"


namespace BZ {


Emitter2D::Emitter2D(const glm::vec2 &positionOffset, uint32 particlesPerSec, float totalLifeSecs,
                     Particle2DRanges &ranges, const Ref<Texture2D> &texture) :
    positionOffset(positionOffset),
    particlesPerSec(particlesPerSec), secsPerParticle(1.0f / particlesPerSec),
    secsUntilNextEmission(1.0f / particlesPerSec), totalLifeSecs(totalLifeSecs), secsToLive(totalLifeSecs),
    texture(texture), ranges(ranges) {
}

void Emitter2D::start() {
    secsUntilNextEmission = 1.0f / particlesPerSec;
    secsToLive = totalLifeSecs;
    activeParticles.clear();
}


/*-------------------------------------------------------------------------------------------*/
ParticleSystem2D::ParticleSystem2D() : position({ 0.0f }) {
}

ParticleSystem2D::ParticleSystem2D(const glm::vec2 &position) : position(position) {
}

void ParticleSystem2D::start() {
    for (auto &emitter : emitters) {
        emitter.start();
    }
    isStarted = true;
}


/*-------------------------------------------------------------------------------------------*/
void ParticleSystem2DSystem::update(const FrameTiming &frameTiming, Scene &scene) {
    scene.getEcsInstance().view<ParticleSystem2D>().each([&frameTiming](auto &particleSystem) {
        if (particleSystem.isStarted) {
            for (auto &emitter : particleSystem.emitters) {

                // Update lifetimes.
                for (auto &particle : emitter.activeParticles) {
                    particle.timeToLiveSecs -= frameTiming.deltaTime.asSeconds();
                }

                auto it = std::remove_if(emitter.activeParticles.begin(), emitter.activeParticles.end(),
                                         [](const Particle2D &particle) { return particle.timeToLiveSecs <= 0.0f; });
                emitter.activeParticles.erase(it, emitter.activeParticles.end());

                // Update current active particles.
                for (auto &particle : emitter.activeParticles) {
                    particle.velocity += particle.acceleration * frameTiming.deltaTime.asSeconds();
                    particle.position += particle.velocity * frameTiming.deltaTime.asSeconds();
                    particle.rotationDeg += particle.angularVelocity * frameTiming.deltaTime.asSeconds();
                    particle.timeToLiveSecs -= frameTiming.deltaTime.asSeconds();
                    particle.tintAndAlpha.a =
                        particle.originalAlpha * (particle.timeToLiveSecs / particle.totalLifeSecs);
                }

                // If not immortal (negative totalLife) and not dead, decrement life.
                if (emitter.totalLifeSecs >= 0.0f && emitter.secsToLive >= 0.0f) {
                    emitter.secsToLive -= frameTiming.deltaTime.asSeconds();
                }

                // If immortal or still alive then emit.
                if (emitter.totalLifeSecs < 0.0f || emitter.secsToLive >= 0.0f) {
                    emitter.secsUntilNextEmission -= frameTiming.deltaTime.asSeconds();
                    if (emitter.secsUntilNextEmission < 0.0f) {
                        uint32 countToEmit =
                            static_cast<uint32>(-emitter.secsUntilNextEmission / emitter.secsPerParticle);
                        if (countToEmit > 0) {
                            // BZ_LOG_DEBUG("emitting {}", countToEmit);
                            for (uint32 i = 0; i < countToEmit; ++i) {
                                emitParticle(particleSystem,  emitter);
                            }

                            // TODO: find out the correct operation
                            // emitter.secsUntilNextEmission += emitter.secsPerParticle;
                            emitter.secsUntilNextEmission = emitter.secsPerParticle;
                        }
                    }
                }
            }
        }
    });
}

void ParticleSystem2DSystem::emitParticle(const ParticleSystem2D &system, Emitter2D &emitter) {
    Particle2D particle;
    particle.position = system.position + emitter.positionOffset + emitter.ranges.positionRange.getValue();
    particle.dimensions = emitter.ranges.dimensionRange.getValue();
    particle.rotationDeg = emitter.ranges.rotationRange.getValue();
    particle.tintAndAlpha = emitter.ranges.tintAndAlphaRange.getValue();
    particle.originalAlpha = particle.tintAndAlpha.a;
    particle.totalLifeSecs = emitter.ranges.lifeSecsRange.getValue();
    particle.timeToLiveSecs = particle.totalLifeSecs;
    particle.velocity = emitter.ranges.velocityRange.getValue();
    particle.angularVelocity = emitter.ranges.angularVelocityRange.getValue();
    particle.acceleration = emitter.ranges.accelerationRange.getValue();
    emitter.activeParticles.push_back(particle);
}
}