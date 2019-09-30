#include "bzpch.h"

#include "Renderer.h"
#include "Bhazel/Renderer/RenderCommand.h"
#include "Bhazel/Renderer/Camera.h"
#include "Bhazel/Application.h"

#include "Bhazel/Renderer/Buffer.h"
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/InputDescription.h"
#include "Bhazel/Renderer/PipelineSettings.h"

#include "Bhazel/Events/WindowEvent.h"


namespace BZ {

    Renderer::API Renderer::api = API::Unknown;

    Renderer::FrameData Renderer::frameData;
    Renderer::InstanceData Renderer::instanceData;

    Ref<Buffer> Renderer::frameConstantBuffer;
    Ref<Buffer> Renderer::instanceConstantBuffer;


    void Renderer::init() {
        //frameConstantBuffer = Buffer::createConstantBuffer(sizeof(frameData));
        //instanceConstantBuffer = Buffer::createConstantBuffer(sizeof(instanceData));

        //TODO: set all pipeline defaults here
        RenderCommand::setRenderMode(RenderMode::Triangles);
        RenderCommand::setBlendingSettings(BlendingSettings(false));
        RenderCommand::setDepthSettings(DepthSettings(false, false));
    }

    void Renderer::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        //frameConstantBuffer.reset();
        //instanceConstantBuffer.reset();
    }

    void Renderer::onWindowResize(WindowResizedEvent &ev) {
        BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    }

    void Renderer::beginScene(Camera &camera, const FrameStats &frameStats) {
        frameData.viewMatrix = camera.getViewMatrix();
        frameData.projectionMatrix = camera.getProjectionMatrix();
        frameData.viewProjectionMatrix = camera.getViewProjectionMatrix();
        frameData.cameraPosition = camera.getPosition();
        frameData.timeAndDelta.x = frameStats.runningTime.asSeconds();
        frameData.timeAndDelta.y = frameStats.lastFrameTime.asSeconds();

        //frameConstantBuffer->setData(&frameData, sizeof(frameData));
        //frameConstantBuffer->bindToPipeline(0);
    }

    void Renderer::endScene() {
    }

    void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix, RenderMode renderMode, uint32 instances) {
        instanceData.modelMatrix = modelMatrix;
        instanceConstantBuffer->setData(&instanceData, sizeof(instanceData));
        instanceConstantBuffer->bindToPipeline(1);

        //TODO we should not set this every draw call
        shader->bindToPipeline();
        inputDescription->bindToPipeline();
        RenderCommand::setRenderMode(renderMode);

        //TODO: this is bad. branching and divisions
        if(inputDescription->hasIndexBuffer())
            RenderCommand::drawInstancedIndexed(inputDescription->getIndexBuffer()->getSize() / sizeof(uint32), instances);
        else {
            auto &vertexBuffer = inputDescription->getVertexBuffers()[0];
            RenderCommand::drawInstanced(vertexBuffer->getSize() / vertexBuffer->getLayout().getStride(), instances);
        }
    }

    void Renderer::submitCompute(const Ref<Shader> &computeShader, uint32 groupsX, uint32 groupsY, uint32 groupsZ, std::initializer_list<Ref<Buffer>> buffers) {
        computeShader->bindToPipeline();

        int unit = 0;
        for(auto &buffer : buffers) {
            buffer->bindToPipelineAsGeneric(unit++);
        }

        RenderCommand::submitCompute(groupsX, groupsY, groupsZ);
    }
}