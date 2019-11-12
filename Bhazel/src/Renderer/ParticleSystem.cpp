#include "bzpch.h"

#include "ParticleSystem.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Texture.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    //TODO: initialization
    BlendingState ParticleSystem::particleBlendState;
    BlendingState ParticleSystem::disableBlendState;

    ParticleSystem::ParticleRanges::ParticleRanges() :
        positionRange(glm::vec3(0.0f)),
        sizeRange(glm::vec3(0.01f, 0.01f, 0.0f), glm::vec3(0.01f, 0.01f, 0.0f)),
        lifeRange(1.0f, 5.0f),
        velocityRange(glm::vec3(-0.25f, 0.25f, 0), glm::vec3(0.25f, 0.5f, 0)),
        accelerationRange(glm::vec3(0.0f, -0.5f, 0), glm::vec3(0.0f)),
        tintRange(glm::vec3(0.2f), glm::vec3(1.0f)) {
    }


    ParticleSystem::ParticleSystem(uint32 particleCount) :
        particleCount(particleCount) {
    }

    void ParticleSystem::init() {
        /*struct Vertex {
            int8 pos[2];
            uint8 texCoord[2];
        };

        //-0.5 and 0.5 after normalization
        const int8 MIN = -64;
        const int8 MAX = 64;

        Vertex vertexBuffer[] = {
            { { MIN, MIN }, {0, 0} },
            { { MAX, MIN }, {1, 0} },
            { { MIN, MAX }, {0, 1} },
            { { MAX, MAX }, {1, 1} },
        };

        DataLayout particleLayout = {
            {DataType::Int8, DataElements::Vec2, "POSITION", true},
            {DataType::Uint8, DataElements::Vec2, "TEXCOORD"},
        };

        particleShader = Shader::create(Graphics::api == Graphics::API::OpenGL ? "shaders/Particle.glsl" : "shaders/Particle.hlsl");
        quadVertexBuffer = Buffer::createVertexBuffer(&vertexBuffer, sizeof(vertexBuffer), particleLayout);
        quadInputDescription = InputDescription::create();
        quadInputDescription->addVertexBuffer(quadVertexBuffer, particleShader);
        computeShader = Shader::create(Graphics::api == Graphics::API::OpenGL ? "shaders/ParticlesCompute.glsl" : "shaders/ParticlesComputes.hlsl");

        std::vector<Particle> particles;
        particles.resize(particleCount);

        for(uint32 i = 0; i < particleCount; ++i) {
            Particle &particle = particles[i];
            particle.positionAndLife.w = -1.0f;
        }

        computeBuffer = Buffer::createVertexBuffer(particles.data(), static_cast<uint32>(particles.size()) * sizeof(Particle), particleLayout);
        constantBuffer = Buffer::createConstantBuffer(sizeof(ranges));
        particleTexture = Texture2D::create("textures/particle.png");*/
    }

    void ParticleSystem::onUpdate() {
        //TODO: only send data if changed
        /*constantBuffer->setData(&ranges, sizeof(ranges));
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
        Renderer::submit(particleShader, quadInputDescription, modelMat, Renderer::PrimitiveTopology::TriangleStrip, particleCount);
        RenderCommand::setBlendingSettings(disableBlendingSettings);*/
    }
}