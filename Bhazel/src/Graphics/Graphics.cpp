#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


namespace BZ {

    Graphics::API Graphics::api = API::Unknown;

    Ref<CommandBuffer> Graphics::commandBuffers[MAX_COMMAND_BUFFERS];
    uint32 Graphics::currentCommandBufferIndex;

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

    GraphicsContext* Graphics::graphicsContext = nullptr;

    uint32 Graphics::currentSceneIndex;
    uint32 Graphics::currentObjectIndex;


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

    uint32 Graphics::beginCommandBuffer() {
        auto &commandBufferRef = graphicsContext->getCurrentFrameCommandBuffer();
        commandBufferRef->resetIndex();

        commandBuffers[currentCommandBufferIndex] = commandBufferRef;
        return currentCommandBufferIndex++;
    }

    void Graphics::clearColorAttachments(uint32 commandBufferId, const ClearValues &clearColor) {       
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearColorAttachments);
        command.clearAttachmentsData.framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer().get();
        command.clearAttachmentsData.clearValue = clearColor;
    }

    void Graphics::clearColorAttachments(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearColor) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearColorAttachments);
        command.clearAttachmentsData.framebuffer = framebuffer.get();
        command.clearAttachmentsData.clearValue = clearColor;
    }

    void Graphics::clearDepthStencilAttachment(uint32 commandBufferId, const ClearValues &clearValue) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearDepthStencilAttachment);
        command.clearAttachmentsData.framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer().get();
        command.clearAttachmentsData.clearValue = clearValue;
    }

    void Graphics::clearDepthStencilAttachment(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearValue) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearDepthStencilAttachment);
        command.clearAttachmentsData.framebuffer = framebuffer.get();
        command.clearAttachmentsData.clearValue = clearValue;
    }

    void Graphics::beginScene(uint32 commandBufferId, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(currentSceneIndex < MAX_SCENES_PER_FRAME, "currentSceneIndex exceeded MAX_SCENES_PER_FRAME!");

        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = viewMatrix;
        sceneConstantBufferData.projectionMatrix = projectionMatrix;
        sceneConstantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;

        uint32 sceneOffset = currentSceneIndex * sizeof(SceneConstantBufferData);
        memcpy(sceneConstantBufferPtr + sceneOffset, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        //TODO: don't really want to bind this every frame, but ImGui shader does not use engine descriptor sets and "unbinds" them when used.
        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = sceneDescriptorSet.get();
        command.bindDescriptorSetData.pipelineState = dummyPipelineState.get();
        command.bindDescriptorSetData.setIndex = BHAZEL_SCENE_DESCRIPTOR_SET_IDX;
        command.bindDescriptorSetData.dynamicBufferOffsets[0] = sceneOffset;
        command.bindDescriptorSetData.dynamicBufferCount = 1;

        BZ_ASSERT_CORE(command.bindDescriptorSetData.dynamicBufferOffsets[0] < sceneConstantBuffer->getRealSize(), "Invalid offset!");

        currentSceneIndex++;
    }

    void Graphics::beginObject(uint32 commandBufferId, const glm::mat4 &modelMatrix) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(currentObjectIndex < MAX_OBJECTS_PER_FRAME, "currentObjectIndex exceeded MAX_OBJECTS_PER_FRAME!");

        ObjectConstantBufferData objectConstantBufferData;
        objectConstantBufferData.modelMatrix = modelMatrix;

        uint32 objectOffset = currentObjectIndex * sizeof(ObjectConstantBufferData);
        memcpy(objectConstantBufferPtr + objectOffset, &objectConstantBufferData, sizeof(ObjectConstantBufferData));

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = objectDescriptorSet.get();
        command.bindDescriptorSetData.pipelineState = dummyPipelineState.get();
        command.bindDescriptorSetData.setIndex = BHAZEL_OBJECT_DESCRIPTOR_SET_IDX;
        command.bindDescriptorSetData.dynamicBufferOffsets[0] = objectOffset;
        command.bindDescriptorSetData.dynamicBufferCount = 1;

        BZ_ASSERT_CORE(command.bindDescriptorSetData.dynamicBufferOffsets[0] < objectConstantBuffer->getRealSize(), "Invalid offset!");

        currentObjectIndex++;
    }

    void Graphics::bindBuffer(uint32 commandBufferId, const Ref<Buffer> &buffer, uint32 offset) {
        BZ_ASSERT_CORE(buffer->getType() == BufferType::Vertex || buffer->getType() == BufferType::Index, "Invalid Buffer type!");
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindBuffer);
        command.bindBufferData.buffer = buffer.get();
        command.bindBufferData.offset = offset;
    }

    void Graphics::bindPipelineState(uint32 commandBufferId, const Ref<PipelineState> &pipelineState) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindPipelineState);
        command.bindPipelineStateData.pipelineState = pipelineState.get();
    }

    /*static void bindDescriptorSets(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet);
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }*/

    void Graphics::bindDescriptorSet(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet, 
                                     const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                     uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) {

        BZ_ASSERT_CORE(dynamicBufferCount < MAX_DESCRIPTOR_DYNAMIC_OFFSETS, "Invalid dynamicBufferCount: {}!", dynamicBufferCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = descriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = setIndex;
        memcpy(command.bindDescriptorSetData.dynamicBufferOffsets, dynamicBufferOffsets, dynamicBufferCount * sizeof(uint32));
        command.bindDescriptorSetData.dynamicBufferCount = dynamicBufferCount;
    }

    void Graphics::draw(uint32 commandBufferId, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::Draw);
        command.drawData.vertexCount = vertexCount;
        command.drawData.instanceCount = instanceCount;
        command.drawData.firstVertex = firstVertex;
        command.drawData.firstInstance = firstInstance;
    }

    void Graphics::drawIndexed(uint32 commandBufferId, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::DrawIndexed);
        command.drawIndexedData.indexCount = indexCount;
        command.drawIndexedData.instanceCount = instanceCount;
        command.drawIndexedData.firstIndex = firstIndex;
        command.drawIndexedData.vertexOffset = vertexOffset;
        command.drawIndexedData.firstInstance = firstInstance;
    }

    void Graphics::setViewports(uint32 commandBufferId, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount) {
        BZ_ASSERT_CORE(viewportCount < MAX_VIEWPORTS, "Invalid viewportCount: {}!", viewportCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::SetViewports);
        command.setViewportsData.firstIndex = firstIndex;
        memcpy(command.setViewportsData.viewports, viewports, viewportCount * sizeof(Viewport));
        command.setViewportsData.viewportCount = viewportCount;
    }

    void Graphics::setScissorRects(uint32 commandBufferId, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount) {
        BZ_ASSERT_CORE(rectCount < MAX_VIEWPORTS, "Invalid rectCount: {}!", rectCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::SetScissorRects);
        command.setScissorRectsData.firstIndex = firstIndex;
        memcpy(command.setScissorRectsData.rects, rects, rectCount * sizeof(ScissorRect));
        command.setScissorRectsData.rectCount = rectCount;
    }

    void Graphics::endCommandBuffer(uint32 commandBufferId) {
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = commandBuffers[commandBufferId];
        commandBuffer->optimizeAndGenerate(graphicsContext->getCurrentFrameFramebuffer());
    }

    void Graphics::waitForDevice() {
        graphicsContext->waitForDevice();
    }

    void Graphics::startFrame() {
        currentCommandBufferIndex = 0;
        currentSceneIndex = 0;
        currentObjectIndex = 0;
    }

    void Graphics::endFrame() {
        graphicsContext->submitCommandBuffersAndFlush(commandBuffers, currentCommandBufferIndex);

        //No need to delete CommandBuffers. The pools are responsible for that.
    }

    void Graphics::onWindowResize(WindowResizedEvent &ev) {
        //TODO: do something to viewport
    }
}