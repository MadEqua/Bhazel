#include "bzpch.h"

#include "Renderer.h"

#include "Bhazel/Renderer/RendererApi.h"

#include "Bhazel/Renderer/Camera.h"
#include "Bhazel/Application.h"

#include "Bhazel/Renderer/Buffer.h"
#include "Bhazel/Renderer/Shader.h"

#include "Bhazel/Events/WindowEvent.h"


namespace BZ {

    Renderer::API Renderer::api = API::Unknown;

    Renderer::ConstantBufferData Renderer::constantBufferData;
    Ref<Buffer> Renderer::constantBuffer;

    RendererApi * Renderer::rendererApi = nullptr;


    void Renderer::init() {
        rendererApi = &Application::getInstance().getGraphicsContext().getRendererAPI();

        constantBuffer = Buffer::createConstantBuffer(sizeof(constantBufferData));
    }

    void Renderer::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        constantBuffer.reset();
    }

    void Renderer::onWindowResize(WindowResizedEvent &ev) {
        //BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    }

    Ref<CommandBuffer> Renderer::startRecording() {
        return rendererApi->startRecording();
    }

    Ref<CommandBuffer> Renderer::startRecordingForFrame(uint32 frameIndex) {
        return rendererApi->startRecordingForFrame(frameIndex);
    }

    //void Renderer::beginScene(Camera &camera, const FrameStats &frameStats) {
    //    frameData.viewMatrix = camera.getViewMatrix();
    //    frameData.projectionMatrix = camera.getProjectionMatrix();
    //    frameData.viewProjectionMatrix = camera.getViewProjectionMatrix();
    //    frameData.cameraPosition = camera.getPosition();
    //    frameData.timeAndDelta.x = frameStats.runningTime.asSeconds();
    //    frameData.timeAndDelta.y = frameStats.lastFrameTime.asSeconds();
    //
    //    //frameConstantBuffer->setData(&frameData, sizeof(frameData));
    //    //frameConstantBuffer->bindToPipeline(0);
    //}

    //void Renderer::endScene() {
    //}

    //void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix, PrimitiveTopology renderMode, uint32 instances) {
    //    instanceData.modelMatrix = modelMatrix;
    //    instanceConstantBuffer->setData(&instanceData, sizeof(instanceData));
    //    //instanceConstantBuffer->bindToPipeline(1);

        //TODO we should not set this every draw call
        //shader->bindToPipeline();
        //inputDescription->bindToPipeline();
        //RenderCommand::setRenderMode(renderMode);

        //TODO: this is bad. branching and divisions
        /*if(inputDescription->hasIndexBuffer())
            RenderCommand::drawInstancedIndexed(inputDescription->getIndexBuffer()->getSize() / sizeof(uint32), instances);
        else {
            auto &vertexBuffer = inputDescription->getVertexBuffers()[0];
            RenderCommand::drawInstanced(vertexBuffer->getSize() / vertexBuffer->getLayout().getSizeBytes(), instances);
        }*/
    //}

    //void Renderer::submitCompute(const Ref<Shader> &computeShader, uint32 groupsX, uint32 groupsY, uint32 groupsZ, std::initializer_list<Ref<Buffer>> buffers) {
        //computeShader->bindToPipeline();

        //int unit = 0;
        /*for(auto &buffer : buffers) {
            buffer->bindToPipelineAsGeneric(unit++);
        }*/

        //RenderCommand::submitCompute(groupsX, groupsY, groupsZ);
    //}
}