#include "bzpch.h"

#include "Renderer.h"
#include "RenderCommand.h"
#include "OrtographicCamera.h"

#include "Buffer.h"
#include "Shader.h"
#include "InputDescription.h"
#include "PipelineSettings.h"


namespace BZ {

    Renderer::API Renderer::api = API::Unknown;

    Renderer::FrameData Renderer::frameData;
    Renderer::InstanceData Renderer::instanceData;

    Ref<Buffer> Renderer::frameConstantBuffer;
    Ref<Buffer> Renderer::instanceConstantBuffer;

    Timer Renderer::timer;


    void Renderer::init() {
        frameConstantBuffer = Buffer::createConstantBuffer(sizeof(frameData));
        instanceConstantBuffer = Buffer::createConstantBuffer(sizeof(instanceData));

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

    void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix, RenderMode renderMode) {
        instanceData.modelMatrix = modelMatrix;
        instanceConstantBuffer->setData(&instanceData, sizeof(instanceData));
        instanceConstantBuffer->bindToPipeline(1);

        //TODO we should not set this every draw call
        shader->bindToPipeline();
        inputDescription->bindToPipeline();
        RenderCommand::setRenderMode(renderMode);

        //TODO: this is bad. branching and divisions.
        if(inputDescription->hasIndexBuffer())
            RenderCommand::drawIndexed(inputDescription->getIndexBuffer()->getSize() / sizeof(uint32));
        else {
            auto &vertexBuffer = inputDescription->getVertexBuffers()[0];
            RenderCommand::draw(vertexBuffer->getSize() / vertexBuffer->getLayout().getStride());
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