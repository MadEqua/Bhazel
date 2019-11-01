#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    //Graphics::FrameConstantBufferData Graphics::frameConstantBufferData;
    //Graphics::ObjectConstantBufferData Graphics::objectConstantBufferData;

    Ref<Buffer> Graphics::frameConstantBuffer;
    Ref<Buffer> Graphics::objectConstantBuffer;

    byte* Graphics::frameConstantBufferPtr = nullptr;
    byte* Graphics::objectConstantBufferPtr  = nullptr;

    Ref<DescriptorSet> Graphics::frameDescriptorSet;
    Ref<DescriptorSet> Graphics::objectDescriptorSet;

    Ref<DescriptorSetLayout> Graphics::descriptorSetLayout;
    
    Ref<PipelineState> Graphics::dummyPipelineState;

    GraphicsContext* Graphics::graphicsContext = nullptr;

    uint32 Graphics::currentObjectIndex = 0;
    bool Graphics::shouldBindFrameDescriptorSet = false;


    void Graphics::init() {
        graphicsContext = &Application::getInstance().getGraphicsContext();

        frameConstantBuffer = Buffer::create(BufferType::Constant, sizeof(FrameConstantBufferData), MemoryType::CpuToGpu);
        objectConstantBuffer = Buffer::create(BufferType::Constant, sizeof(ObjectConstantBufferData) * MAX_OBJECTS_PER_FRAME, MemoryType::CpuToGpu);

        //Keep the full buffers mapped. This puts the responsability of computing offsets on this class.
        //TODO: This works because this function is called on frame 0, so the replica offset returned is 0. Not OK.
        frameConstantBufferPtr = frameConstantBuffer->map(0);
        objectConstantBufferPtr = objectConstantBuffer->map(0);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(BZ::DescriptorType::ConstantBufferDynamic, BZ::flagsToMask(BZ::ShaderStageFlags::All), 1);
        descriptorSetLayout = descriptorSetLayoutBuilder.build();

        frameDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        frameDescriptorSet->setConstantBuffer(frameConstantBuffer, 0, 0, sizeof(FrameConstantBufferData));

        objectDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        objectDescriptorSet->setConstantBuffer(objectConstantBuffer, 0, 0, sizeof(ObjectConstantBufferData));

        PipelineStateData pipelineStateData;
        pipelineStateData.primitiveTopology = BZ::PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, 1.0f, 1.0f} };
        pipelineStateData.blendingState.attachmentBlendingStates = { {} };
        pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
        pipelineStateData.framebuffer = BZ::Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("dummy");
        shaderBuilder.fromBinaryFile(BZ::ShaderStage::Vertex, "shaders/bin/dummyVert.spv");
        shaderBuilder.fromBinaryFile(BZ::ShaderStage::Fragment, "shaders/bin/dummyFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        dummyPipelineState = PipelineState::create(pipelineStateData);
    }

    void Graphics::destroy() {
        frameConstantBuffer->unmap();
        objectConstantBuffer->unmap();

        //Destroy this 'manually' to avoid the static destruction lottery
        frameConstantBuffer.reset();
        objectConstantBuffer.reset();
        frameDescriptorSet.reset();
        objectDescriptorSet.reset();
        descriptorSetLayout.reset();
        dummyPipelineState.reset();
    }

    Ref<CommandBuffer> Graphics::startRecording() {
        return graphicsContext->startRecording();
    }

    Ref<CommandBuffer> Graphics::startRecording(const Ref<Framebuffer> &framebuffer) {
        return graphicsContext->startRecording(framebuffer);
    }

    void Graphics::startScene(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        FrameConstantBufferData frameConstantBufferData;
        frameConstantBufferData.viewMatrix = viewMatrix;
        frameConstantBufferData.projectionMatrix = projectionMatrix;
        frameConstantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        memcpy(frameConstantBufferPtr + frameConstantBuffer->getBaseOfReplicaOffset(), &frameConstantBufferData, sizeof(FrameConstantBufferData));

        currentObjectIndex = 0;

        //The first scene of the frame will bind frame DescriptorSets.
        //if(shouldBindFrameDescriptorSet) { //TODO: check this logic
            uint32 currentFrameBase = sizeof(FrameConstantBufferData) * graphicsContext->getCurrentFrameIndex();
            graphicsContext->bindDescriptorSet(commandBuffer, frameDescriptorSet, dummyPipelineState, BHAZEL_FRAME_DESCRIPTOR_SET_IDX, &currentFrameBase, 1);
            shouldBindFrameDescriptorSet = false;
        //}
    }

    void Graphics::startObject(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &modelMatrix) {
        BZ_ASSERT_CORE(currentObjectIndex < MAX_OBJECTS_PER_FRAME, "currentObjectIndex exceeded MAX_OBJECTS_PER_FRAME!");

        ObjectConstantBufferData objectConstantBufferData;
        objectConstantBufferData.modelMatrix = modelMatrix;

        uint32 objectOffset = currentObjectIndex * sizeof(ObjectConstantBufferData);
        memcpy(objectConstantBufferPtr + objectConstantBuffer->getBaseOfReplicaOffset() + objectOffset, &objectConstantBufferData, sizeof(ObjectConstantBufferData));

        uint32 currentFrameBase = MAX_OBJECTS_PER_FRAME * sizeof(ObjectConstantBufferData) * graphicsContext->getCurrentFrameIndex();
        uint32 totalOffset = currentFrameBase + currentObjectIndex * sizeof(ObjectConstantBufferData);
        graphicsContext->bindDescriptorSet(commandBuffer, objectDescriptorSet, dummyPipelineState, BHAZEL_OBJECT_DESCRIPTOR_SET_IDX, &totalOffset, 1);
    }

    void Graphics::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindVertexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        graphicsContext->bindIndexBuffer(commandBuffer, buffer);
    }

    void Graphics::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
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

    void Graphics::waitForDevice() {
        graphicsContext->waitForDevice();
    }

    void Graphics::startFrame() {
        shouldBindFrameDescriptorSet = true;
    }

    void Graphics::endFrame() {
        graphicsContext->endFrame();
    }

    void Graphics::onWindowResize(WindowResizedEvent &ev) {
        //BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    }
}