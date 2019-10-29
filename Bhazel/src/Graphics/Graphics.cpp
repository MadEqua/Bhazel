#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Buffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/PipelineState.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    //Graphics::FrameConstantBufferData Graphics::frameConstantBufferData;
    //Graphics::ObjectConstantBufferData Graphics::objectConstantBufferData;

    Ref<Buffer> Graphics::frameConstantBuffer;
    Ref<Buffer> Graphics::objectConstantBuffer;

    Ref<DescriptorSet> Graphics::frameDescriptorSet;
    Ref<DescriptorSet> Graphics::objectDescriptorSet;

    Ref<DescriptorSetLayout> Graphics::descriptorSetLayout;

    GraphicsContext* Graphics::graphicsContext = nullptr;

    uint32 Graphics::currentObjectIndex = 0;


    void Graphics::init() {
        graphicsContext = &Application::getInstance().getGraphicsContext();

        frameConstantBuffer = Buffer::createConstantBuffer(sizeof(FrameConstantBufferData), true);
        objectConstantBuffer = Buffer::createConstantBuffer(sizeof(ObjectConstantBufferData) * MAX_OBJECTS_PER_FRAME, true);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(BZ::DescriptorType::ConstantBufferDynamic, BZ::flagsToMask(BZ::ShaderStageFlags::All), 1);
        descriptorSetLayout = descriptorSetLayoutBuilder.build();

        frameDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        frameDescriptorSet->setConstantBuffer(frameConstantBuffer, 0, 0, sizeof(FrameConstantBufferData));

        objectDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        objectDescriptorSet->setConstantBuffer(objectConstantBuffer, 0, 0, sizeof(ObjectConstantBufferData));
    }

    void Graphics::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        frameConstantBuffer.reset();
        objectConstantBuffer.reset();
        frameDescriptorSet.reset();
        objectDescriptorSet.reset();
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

    void Graphics::startScene(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        FrameConstantBufferData frameConstantBufferData;
        frameConstantBufferData.viewMatrix = viewMatrix;
        frameConstantBufferData.projectionMatrix = projectionMatrix;
        frameConstantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        frameConstantBuffer->setData(&frameConstantBufferData, 0, sizeof(FrameConstantBufferData));

        currentObjectIndex = 0;
    }

    void Graphics::startObject(const glm::mat4 &modelMatrix) {
        BZ_ASSERT_CORE(currentObjectIndex < MAX_OBJECTS_PER_FRAME, "currentObjectIndex exceeded MAX_OBJECTS_PER_FRAME!");

        ObjectConstantBufferData objectConstantBufferData;
        objectConstantBufferData.modelMatrix = modelMatrix;

        uint32 objectOffset = currentObjectIndex * sizeof(ObjectConstantBufferData);
        objectConstantBuffer->setData(&objectConstantBufferData, objectOffset, sizeof(ObjectConstantBufferData));
    }

    void Graphics::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindVertexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindIndexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {

        //Always bind the engine descriptor set, it's per frame dynamic.
        //TODO: this should only be bound once per frame, not here.
        uint32 currentFrameBase = sizeof(FrameConstantBufferData) * graphicsContext->getCurrentFrameIndex();
        graphicsContext->bindDescriptorSet(commandBuffer, frameDescriptorSet, pipelineState, 0, &currentFrameBase, 1);

        //TODO: this does not belong here. The "problem" to move it is pipelineState.
        currentFrameBase = MAX_OBJECTS_PER_FRAME * sizeof(ObjectConstantBufferData) * graphicsContext->getCurrentFrameIndex();
        uint32 totalOffset = currentFrameBase + currentObjectIndex * sizeof(ObjectConstantBufferData);
        graphicsContext->bindDescriptorSet(commandBuffer, objectDescriptorSet, pipelineState, 1, &totalOffset, 1);

        graphicsContext->bindPipelineState(commandBuffer, pipelineState);
    }

    /*static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }*/

    void Graphics::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, 
                                     const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                     uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) {
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState, setIndex, dynamicBufferOffsets, dynamicBufferCount);
    }

    void Graphics::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        graphicsContext->draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void Graphics::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        graphicsContext->drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void Graphics::endObject() {
        currentObjectIndex++;
    }

    void Graphics::endScene() {
    }

    void Graphics::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        graphicsContext->endRecording(commandBuffer);
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