#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Buffer.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    Graphics::ConstantBufferData Graphics::constantBufferData;
    Ref<Buffer> Graphics::constantBuffer;
    Ref<DescriptorSet> Graphics::descriptorSet;
    Ref<DescriptorSetLayout> Graphics::descriptorSetLayout;

    GraphicsContext * Graphics::graphicsContext = nullptr;


    void Graphics::init() {
        graphicsContext = &Application::getInstance().getGraphicsContext();
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
        return graphicsContext->startRecording();
    }

    Ref<CommandBuffer> Graphics::startRecording(const Ref<Framebuffer> &framebuffer) {
        return graphicsContext->startRecording(framebuffer);
    }

    Ref<CommandBuffer> Graphics::startRecordingForFrame(uint32 frameIndex) {
        return graphicsContext->startRecordingForFrame(frameIndex);
    }

    Ref<CommandBuffer> Graphics::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {
        return graphicsContext->startRecordingForFrame(frameIndex, framebuffer);
    }

    void Graphics::startObject(const glm::mat4 &modelMatrix) {
        constantBufferData.modelMatrix = modelMatrix;
        //uint32 objectOffset = static_cast<void*>(&constantBufferData.modelMatrix) - static_cast<void*>(&constantBufferData);

        constantBuffer->setData(&constantBufferData, sizeof(ConstantBufferData), 0); //TODO: set only the object part of the buffer
    }

    void Graphics::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindVertexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindIndexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState); //Always bind the engine descriptor set. TODO: can this be done only once?
        graphicsContext->bindPipelineState(commandBuffer, pipelineState);
    }

    /*static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }*/

    void Graphics::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }

    void Graphics::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        graphicsContext->draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void Graphics::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        graphicsContext->drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void Graphics::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        graphicsContext->endRecording(commandBuffer);
    }

    void Graphics::startFrame(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        constantBufferData.viewMatrix = viewMatrix;
        constantBufferData.projectionMatrix = projectionMatrix;
        constantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        constantBuffer->setData(&constantBufferData, sizeof(ConstantBufferData), 0); //TODO: set only the frame part of the buffer
    }

    void Graphics::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        graphicsContext->submitCommandBuffer(commandBuffer);
    }

    void Graphics::endFrame() {
        graphicsContext->endFrame();
    }

    void Graphics::waitForDevice() {
        graphicsContext->waitForDevice();
    }
}