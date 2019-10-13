#include "bzpch.h"

#include "Renderer.h"

#include "Bhazel/Renderer/RendererApi.h"

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

    Ref<CommandBuffer> Renderer::startRecording(const Ref<Framebuffer> &framebuffer) {
        return rendererApi->startRecording(framebuffer);
    }

    Ref<CommandBuffer> Renderer::startRecordingForFrame(uint32 frameIndex) {
        return rendererApi->startRecordingForFrame(frameIndex);
    }

    Ref<CommandBuffer> Renderer::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {
        return rendererApi->startRecordingForFrame(frameIndex, framebuffer);
    }

    void Renderer::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        rendererApi->bindVertexBuffer(commandBuffer, buffer);
    }

    void Renderer::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        rendererApi->bindIndexBuffer(commandBuffer, buffer);
    }

    void Renderer::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        rendererApi->bindPipelineState(commandBuffer, pipelineState);
    }

    void Renderer::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        rendererApi->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }

    void Renderer::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        rendererApi->draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void Renderer::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        rendererApi->drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void Renderer::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        rendererApi->endRecording(commandBuffer);
    }

    void Renderer::startFrame() {
        //TODO
    }

    void Renderer::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        rendererApi->submitCommandBuffer(commandBuffer);
    }

    void Renderer::endFrame() {
        rendererApi->endFrame();
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