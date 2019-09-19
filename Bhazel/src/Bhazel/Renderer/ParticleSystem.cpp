#include "bzpch.h"

#include "ParticleSystem.h"
#include "Buffer.h"
#include "InputDescription.h"
#include "Shader.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    ParticleSystem::ParticleRanges::ParticleRanges() :
        positionRange(glm::vec3(0.0f)),
        sizeRange(glm::vec3(0.01f, 0.01f, 0.0f), glm::vec3(0.01f, 0.01f, 0.0f)),
        lifeRange(1.0f, 5.0f),
        velocityRange(glm::vec3(-0.25f, 0.25f, 0), glm::vec3(0.25f, 0.5f, 0)),
        accelerationRange(glm::vec3(0.0f, -0.5f, 0), glm::vec3(0.0f)),
        tintRange(glm::vec3(0.2f), glm::vec3(1.0f)) {
    }


    ParticleSystem::ParticleSystem(uint32 particleCount) :
        particleCount(particleCount), 
        position(glm::vec3(0.0f)), 
        scale(glm::vec3(1.0f)),
        eulerAngles(glm::vec3(0.0f)),
        particleBlendingSettings({BlendingFunction::SourceAlpha, BlendingFunction::OneMinusSourceAlpha, BlendingEquation::Add}),
        disableBlendingSettings(false) {
    }

    void ParticleSystem::init() {

        //TODO: all particle systems will have a buffer with this data. not good!
        float quadVertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.0f, 0.0f,
            
            0.5f, -0.5f, 0.0f,
            1.0f, 0.0f,

            -0.5f, 0.5f, 0.0f,
            0.0f, 1.0f,

            0.5f, 0.5f, 0.0f,
            1.0f, 1.0f
        };

        BufferLayout particleLayout = {
            {DataType::Vec3, "POSITION"},
            {DataType::Vec2, "TEXCOORD"},
        };

        particleShader = Shader::create(Renderer::api == Renderer::API::OpenGL ? "assets/shaders/Particle.glsl" : "assets/shaders/Particle.hlsl");

        quadVertexBuffer = Buffer::createVertexBuffer(quadVertices, sizeof(quadVertices), particleLayout);
        quadInputDescription = InputDescription::create();
        quadInputDescription->addVertexBuffer(quadVertexBuffer, particleShader);

        std::vector<Particle> particles;
        particles.resize(particleCount);

        for(uint32 i = 0; i < particleCount; ++i) {
            Particle &particle = particles[i];
            particle.positionAndLife.w = -1.0f;
        }
        computeBuffer = Buffer::createVertexBuffer(particles.data(), static_cast<uint32>(particles.size()) * sizeof(Particle), particleLayout);
        computeShader = Shader::create(Renderer::api == Renderer::API::OpenGL ? "assets/shaders/Compute.glsl" : "assets/shaders/Compute.hlsl");

        constantBuffer = Buffer::createConstantBuffer(sizeof(ranges));

        particleTexture = Texture2D::create("assets/textures/alphatest.png");
    }

    void ParticleSystem::render() {
        //TODO: only send data if changed
        constantBuffer->setData(&ranges, sizeof(ranges));
        constantBuffer->bindToPipeline(2);

        glm::mat4 modelMat(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::rotate(modelMat, glm::radians(eulerAngles.y), glm::vec3(0, 1, 0));
        modelMat = glm::rotate(modelMat, glm::radians(eulerAngles.x), glm::vec3(1, 0, 0));
        modelMat = glm::rotate(modelMat, glm::radians(eulerAngles.z), glm::vec3(0, 0, 1));
        modelMat = glm::scale(modelMat, scale);

        Renderer::submitCompute(computeShader, particleCount / WORK_GROUP_SIZE, 1, 1, {computeBuffer});

        particleTexture->bindToPipeline(0);
        RenderCommand::setBlendingSettings(particleBlendingSettings); //TODO: not sure if RenderCommands belong here
        Renderer::submit(particleShader, quadInputDescription, modelMat, Renderer::RenderMode::TriangleStrip, particleCount);
        RenderCommand::setBlendingSettings(disableBlendingSettings);
    }
}