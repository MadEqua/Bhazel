#include "bzpch.h"

#include "Graphics.h"

#include "Graphics/GraphicsApi.h"

#include "Core/Application.h"

#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/DescriptorSet.h"

#include "Events/WindowEvent.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    Graphics::ConstantBufferData Graphics::constantBufferData;
    Ref<Buffer> Graphics::constantBuffer;
    Ref<DescriptorSet> Graphics::descriptorSet;
    Ref<DescriptorSetLayout> Graphics::descriptorSetLayout;

    GraphicsApi * Graphics::graphicsApi = nullptr;


    void Graphics::init() {
        graphicsApi = &Application::getInstance().getGraphicsContext().getGraphicsAPI();
        constantBuffer = Buffer::createConstantBuffer(sizeof(constantBufferData));

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(BZ::DescriptorType::ConstantBuffer, BZ::flagsToMask(BZ::ShaderStageFlags::All), 1);
        descriptorSetLayout = descriptorSetLayoutBuilder.build();

        descriptorSet = DescriptorSet::create(descriptorSetLayout);
        descriptorSet->setConstantBuffer(constantBuffer, 0, 0, sizeof(constantBufferData));
    }

    void Graphics::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        constantBuffer.reset();
        descriptorSet.reset();
        descriptorSetLayout.reset();
    }

    void Graphics::onWindowResize(WindowResizedEvent &ev) {
        //BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    }

    Ref<CommandBuffer> Graphics::startRecording() {
        return graphicsApi->startRecording();
    }

    Ref<CommandBuffer> Graphics::startRecording(const Ref<Framebuffer> &framebuffer) {
        return graphicsApi->startRecording(framebuffer);
    }

    Ref<CommandBuffer> Graphics::startRecordingForFrame(uint32 frameIndex) {
        return graphicsApi->startRecordingForFrame(frameIndex);
    }

    Ref<CommandBuffer> Graphics::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {
        return graphicsApi->startRecordingForFrame(frameIndex, framebuffer);
    }

    void Graphics::startObject(const glm::mat4 &modelMatrix) {
        constantBufferData.modelMatrix = modelMatrix;
        //uint32 objectOffset = static_cast<void*>(&constantBufferData.modelMatrix) - static_cast<void*>(&constantBufferData);

        constantBuffer->setData(&constantBufferData, sizeof(ConstantBufferData), 0); //TODO: set only the object part of the buffer
    }

    void Graphics::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsApi->bindVertexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsApi->bindIndexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        graphicsApi->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState); //Always bind the engine descriptor set. TODO: can this be done only once?
        graphicsApi->bindPipelineState(commandBuffer, pipelineState);
    }

    /*static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        graphicsApi->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }*/

    void Graphics::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        graphicsApi->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }

    void Graphics::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        graphicsApi->draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void Graphics::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        graphicsApi->drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void Graphics::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        graphicsApi->endRecording(commandBuffer);
    }

    void Graphics::startFrame(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        constantBufferData.viewMatrix = viewMatrix;
        constantBufferData.projectionMatrix = projectionMatrix;
        constantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        constantBuffer->setData(&constantBufferData, sizeof(ConstantBufferData), 0); //TODO: set only the frame part of the buffer
    }

    void Graphics::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        graphicsApi->submitCommandBuffer(commandBuffer);
    }

    void Graphics::endFrame() {
        graphicsApi->endFrame();
    }

    void Graphics::waitForDevice() {
        graphicsApi->waitForDevice();
    }






    //void Graphics::beginScene(Camera &camera, const FrameStats &frameStats) {
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

    //void Graphics::endScene() {
    //}

    //void Graphics::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix, PrimitiveTopology renderMode, uint32 instances) {
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

    //void Graphics::submitCompute(const Ref<Shader> &computeShader, uint32 groupsX, uint32 groupsY, uint32 groupsZ, std::initializer_list<Ref<Buffer>> buffers) {
        //computeShader->bindToPipeline();

        //int unit = 0;
        /*for(auto &buffer : buffers) {
            buffer->bindToPipelineAsGeneric(unit++);
        }*/

        //RenderCommand::submitCompute(groupsX, groupsY, groupsZ);
    //}
}