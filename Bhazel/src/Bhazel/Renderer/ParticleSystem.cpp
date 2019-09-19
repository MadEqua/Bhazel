#include "bzpch.h"

#include "ParticleSystem.h"
#include "Buffer.h"
#include "InputDescription.h"
#include "Shader.h"
#include "Renderer.h"


namespace BZ {

    ParticleSystem::ParticleRanges::ParticleRanges() :
        positionRange(glm::vec3(0.0f)),
        lifeRange(1.0f, 5.0f),
        velocityRange(glm::vec3(-0.1f, 0.25f, 0), glm::vec3(0.1f, 0.5f, 0)),
        accelerationRange(glm::vec3(0.0f, -0.5f, 0), glm::vec3(0.0f)),
        tintRange(glm::vec3(0.2f), glm::vec3(1.0f)) {
    }


    ParticleSystem::ParticleSystem(uint32 particleCount) :
        particleCount(particleCount) {
    }

    void ParticleSystem::init() {
        std::vector<Particle> particles;
        particles.resize(particleCount);

        for(int i = 0; i < particleCount; ++i) {
            Particle &particle = particles[i];
            particle.positionAndLife.w = -1.0f;
        }


        //DataTypes taking into account the alignments used for binding the buffer on the compute shader.
        BufferLayout particleLayout = {
            {DataType::Vec4, "POSITION"},
            {DataType::Vec4, "VEL"},
            {DataType::Vec4, "ACCEL"},
            {DataType::Vec4, "TINT"}
        };

        buffer = Buffer::createVertexBuffer(particles.data(), static_cast<uint32>(particles.size()) * sizeof(Particle), particleLayout);

        computeShader = Shader::create(Renderer::api == Renderer::API::OpenGL ? "assets/shaders/Compute.glsl" : "assets/shaders/Compute.hlsl");
        particleShader = Shader::create(Renderer::api == Renderer::API::OpenGL ? "assets/shaders/Particle.glsl" : "assets/shaders/Particle.hlsl");

        inputDescription = InputDescription::create();
        inputDescription->addVertexBuffer(buffer, particleShader);

        constantBuffer = Buffer::createConstantBuffer(sizeof(ranges));
    }

    void ParticleSystem::render(const glm::vec3 &position) {
        //TODO: only send data if changed
        constantBuffer->setData(&ranges, sizeof(ranges));
        constantBuffer->bindToPipeline(2);

        glm::mat4 modelMat(1.0f);
        modelMat[3][0] = position.x;
        modelMat[3][1] = position.y;
        modelMat[3][2] = position.z;

        Renderer::submitCompute(computeShader, particleCount / WORK_GROUP_SIZE, 1, 1, {buffer});
        Renderer::submit(particleShader, inputDescription, modelMat, BZ::Renderer::RenderMode::Points);
    }
}