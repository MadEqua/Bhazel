#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    //Graphics::FrameConstantBufferData Graphics::frameConstantBufferData;
    //Graphics::ObjectConstantBufferData Graphics::objectConstantBufferData;

    Ref<Buffer> Graphics::frameConstantBuffer;
    Ref<Buffer> Graphics::sceneConstantBuffer;
    Ref<Buffer> Graphics::objectConstantBuffer;

    BufferPtr Graphics::frameConstantBufferPtr;
    BufferPtr Graphics::sceneConstantBufferPtr;
    BufferPtr Graphics::objectConstantBufferPtr;

    Ref<DescriptorSet> Graphics::frameDescriptorSet;
    Ref<DescriptorSet> Graphics::sceneDescriptorSet;
    Ref<DescriptorSet> Graphics::objectDescriptorSet;

    Ref<DescriptorSetLayout> Graphics::descriptorSetLayout;
    
    Ref<PipelineState> Graphics::dummyPipelineState;

    std::vector<Ref<CommandBuffer>> Graphics::pendingCommandBuffers;

    GraphicsContext* Graphics::graphicsContext = nullptr;

    uint32 Graphics::currentSceneIndex = 0;
    uint32 Graphics::currentObjectIndex = 0;


    void Graphics::init() {
        graphicsContext = &Application::getInstance().getGraphicsContext();

        frameConstantBuffer = Buffer::create(BufferType::Constant, sizeof(FrameConstantBufferData), MemoryType::CpuToGpu);
        sceneConstantBuffer = Buffer::create(BufferType::Constant, sizeof(SceneConstantBufferData) * MAX_SCENES_PER_FRAME, MemoryType::CpuToGpu);
        objectConstantBuffer = Buffer::create(BufferType::Constant, sizeof(ObjectConstantBufferData) * MAX_OBJECTS_PER_FRAME, MemoryType::CpuToGpu);

        frameConstantBufferPtr = frameConstantBuffer->map(0);
        sceneConstantBufferPtr = sceneConstantBuffer->map(0);
        objectConstantBufferPtr = objectConstantBuffer->map(0);

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::All), 1);
        descriptorSetLayout = descriptorSetLayoutBuilder.build();

        frameDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        frameDescriptorSet->setConstantBuffer(frameConstantBuffer, 0, 0, sizeof(FrameConstantBufferData));

        sceneDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        sceneDescriptorSet->setConstantBuffer(sceneConstantBuffer, 0, 0, sizeof(SceneConstantBufferData));

        objectDescriptorSet = DescriptorSet::create(descriptorSetLayout);
        objectDescriptorSet->setConstantBuffer(objectConstantBuffer, 0, 0, sizeof(ObjectConstantBufferData));

        PipelineStateData pipelineStateData;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, 1.0f, 1.0f } };
        pipelineStateData.scissorRects = { { 0u, 0u, 1u, 1u } };
        pipelineStateData.blendingState.attachmentBlendingStates = { {} };
        pipelineStateData.descriptorSetLayouts = { descriptorSetLayout };
        Shader::Builder shaderBuilder;
        shaderBuilder.setName("dummy");
        shaderBuilder.fromBinaryFile(ShaderStage::Vertex, "shaders/bin/dummyVert.spv");
        shaderBuilder.fromBinaryFile(ShaderStage::Fragment, "shaders/bin/dummyFrag.spv");
        pipelineStateData.shader = shaderBuilder.build();

        dummyPipelineState = PipelineState::create(pipelineStateData);

        pendingCommandBuffers.reserve(64);
    }

    void Graphics::destroy() {
        frameConstantBuffer->unmap();
        sceneConstantBuffer->unmap();
        objectConstantBuffer->unmap();

        //Destroy this 'manually' to avoid the static destruction lottery
        frameConstantBuffer.reset();
        sceneConstantBuffer.reset();
        objectConstantBuffer.reset();

        frameDescriptorSet.reset();
        sceneDescriptorSet.reset();
        objectDescriptorSet.reset();

        descriptorSetLayout.reset();
        dummyPipelineState.reset();
    }

    Ref<CommandBuffer> Graphics::startRecording() {
        auto &framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();
        return graphicsContext->startRecording(framebuffer);
    }

    Ref<CommandBuffer> Graphics::startRecording(const Ref<Framebuffer> &framebuffer) {
        return graphicsContext->startRecording(framebuffer);
    }

    void Graphics::clearColorAttachments(const Ref<CommandBuffer> &commandBuffer, const Ref<Framebuffer> &framebuffer, const ClearValues &clearColor) {
        graphicsContext->clearColorAttachments(commandBuffer, framebuffer, clearColor);
    }

    void Graphics::clearColorAttachments(const Ref<CommandBuffer> &commandBuffer, const ClearValues &clearColor) {
        auto &framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();
        graphicsContext->clearColorAttachments(commandBuffer, framebuffer, clearColor);
    }

    void Graphics::clearDepthStencilAttachments(const Ref<CommandBuffer> &commandBuffer, const ClearValues &clearValue) {
        auto &framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();
        graphicsContext->clearDepthStencilAttachments(commandBuffer, framebuffer, clearValue);
    }

    void Graphics::clearDepthStencilAttachments(const Ref<CommandBuffer> &commandBuffer, const Ref<Framebuffer> &framebuffer, const ClearValues &clearValue) {
        graphicsContext->clearDepthStencilAttachments(commandBuffer, framebuffer, clearValue);
    }

    void Graphics::startScene(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        BZ_ASSERT_CORE(currentSceneIndex < MAX_SCENES_PER_FRAME, "currentSceneIndex exceeded MAX_SCENES_PER_FRAME!");

        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = viewMatrix;
        sceneConstantBufferData.projectionMatrix = projectionMatrix;
        sceneConstantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        uint32 sceneOffset = currentSceneIndex * sizeof(SceneConstantBufferData);
        memcpy(sceneConstantBufferPtr + sceneOffset, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        //TODO: don't really want to bind this every frame, but ImGui shader does not use engine descriptor sets and "unbinds" them when used.
        uint32 totalOffset = sceneConstantBuffer->getCurrentBaseOfReplicaOffset() + sceneOffset;
        graphicsContext->bindDescriptorSet(commandBuffer, sceneDescriptorSet, dummyPipelineState, BHAZEL_SCENE_DESCRIPTOR_SET_IDX, &totalOffset, 1);
    }

    void Graphics::startObject(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &modelMatrix) {
        BZ_ASSERT_CORE(currentObjectIndex < MAX_OBJECTS_PER_FRAME, "currentObjectIndex exceeded MAX_OBJECTS_PER_FRAME!");

        ObjectConstantBufferData objectConstantBufferData;
        objectConstantBufferData.modelMatrix = modelMatrix;

        uint32 objectOffset = currentObjectIndex * sizeof(ObjectConstantBufferData);
        memcpy(objectConstantBufferPtr + objectOffset, &objectConstantBufferData, sizeof(ObjectConstantBufferData));

        uint32 totalOffset = objectConstantBuffer->getCurrentBaseOfReplicaOffset() + objectOffset;
        graphicsContext->bindDescriptorSet(commandBuffer, objectDescriptorSet, dummyPipelineState, BHAZEL_OBJECT_DESCRIPTOR_SET_IDX, &totalOffset, 1);
    }

    void Graphics::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) {
        graphicsContext->bindVertexBuffer(commandBuffer, buffer, buffer->getCurrentBaseOfReplicaOffset() + offset);
    }

    void Graphics::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) {
        graphicsContext->bindIndexBuffer(commandBuffer, buffer, buffer->getCurrentBaseOfReplicaOffset() + offset);
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

        //Mix correctly the dynamicBufferOffsets coming from the user with the ones that the engine needs to send behind the scenes for dynamic buffers.
        uint32 finalDynamicBufferOffsets[32];
        uint32 dynIndex = 0;
        uint32 userDynIndex = 0;
        uint32 binding = 0;
        for(const auto &desc : descriptorSet->getLayout()->getDescriptorDescs()) {
            if(desc.type == DescriptorType::ConstantBufferDynamic || desc.type == DescriptorType::StorageBufferDynamic) {
                const auto *dynBufferData = descriptorSet->getDynamicBufferDataByBinding(binding);
                BZ_ASSERT_CORE(dynBufferData, "Non-existent binding should not happen!");
                finalDynamicBufferOffsets[dynIndex] = dynBufferData->buffer->getCurrentBaseOfReplicaOffset();
                if(!dynBufferData->isAutoAddedByEngine) {
                    finalDynamicBufferOffsets[dynIndex] += dynamicBufferOffsets[userDynIndex++];
                }
                dynIndex++;
            }
            binding++;
        }

        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState, setIndex, finalDynamicBufferOffsets, dynIndex);
    }

    void Graphics::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        graphicsContext->draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void Graphics::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        graphicsContext->drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void Graphics::setViewports(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount) {
        graphicsContext->setViewports(commandBuffer, firstIndex, viewports, viewportCount);
    }

    void Graphics::setScissorRects(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount) {
        graphicsContext->setScissorRects(commandBuffer, firstIndex, rects, rectCount);
    }

    void Graphics::endObject() {
        currentObjectIndex++;
    }

    void Graphics::endScene() {
        currentSceneIndex++;
    }

    void Graphics::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        graphicsContext->endRecording(commandBuffer);
    }

    void Graphics::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        pendingCommandBuffers.push_back(commandBuffer);
    }

    void Graphics::waitForDevice() {
        graphicsContext->waitForDevice();
    }

    void Graphics::startFrame() {
        currentSceneIndex = 0;
        currentObjectIndex = 0;
    }

    void Graphics::endFrame() {
        graphicsContext->submitCommandBuffersAndFlush(pendingCommandBuffers);
        pendingCommandBuffers.clear();
    }

    void Graphics::onWindowResize(WindowResizedEvent &ev) {
        //BZ::RenderCommand::setViewport(0, 0, ev.getWidth(), ev.getHeight());
    }
}