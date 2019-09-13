#include "bzpch.h"

#include "Renderer.h"
#include "RenderCommand.h"
#include "OrtographicCamera.h"

#include "Shader.h"
#include "InputDescription.h"
#include "PipelineSettings.h"


namespace BZ {

    Renderer::API Renderer::api = API::Unknown;

    Renderer::FrameData Renderer::frameData;
    Renderer::InstanceData Renderer::instanceData;

    Ref<ConstantBuffer> Renderer::frameConstantBuffer;
    Ref<ConstantBuffer> Renderer::instanceConstantBuffer;

    Timer Renderer::timer;


    void Renderer::init() {
        frameConstantBuffer = ConstantBuffer::create(sizeof(frameData));
        instanceConstantBuffer = ConstantBuffer::create(sizeof(instanceData));

        //TODO: set all pipeline defaults here
        RenderCommand::setRenderMode(RenderMode::Triangles);
        RenderCommand::setBlendingSettings(BlendingSettings(false));
        RenderCommand::setDepthSettings(DepthSettings(false, false));

        timer.start();
    }

    void Renderer::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        frameConstantBuffer.reset();
        instanceConstantBuffer.reset();
    }

    void Renderer::beginScene(OrtographicCamera &camera) {
        frameData.viewMatrix = camera.getViewMatrix();
        frameData.projectionMatrix = camera.getProjectionMatrix();
        frameData.viewProjectionMatrix = camera.getViewProjectionMatrix();
        frameData.runningTime = timer.getElapsedTime().asSeconds();

        frameConstantBuffer->setData(&frameData, sizeof(frameData));
        frameConstantBuffer->bindToPipeline(0);
    }

    void Renderer::endScene() {
    }

    void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix) {
        instanceData.modelMatrix = modelMatrix;
        instanceConstantBuffer->setData(&instanceData, sizeof(instanceData));
        instanceConstantBuffer->bindToPipeline(1);

        shader->bindToPipeline();

        inputDescription->bindToPipeline();
        RenderCommand::drawIndexed(inputDescription->getIndexBuffer()->getCount());
    }
}