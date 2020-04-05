#include "bzpch.h"

#include "Graphics.h"

#include "Constants.h"
#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"
#include "Renderer/Scene.h"
#include "Renderer/Material.h"


namespace BZ {  

    struct alignas(256) FrameConstantBufferData { //TODO: check this align value
        glm::vec2 timeAndDelta;
    };

    struct alignas(256) SceneConstantBufferData { //TODO: check this align value
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewProjectionMatrix;
        glm::vec4 cameraPosition; //vec4 to simplify alignments
        glm::vec4 dirLightsDirectionsAndIntensities[MAX_DIR_LIGHTS_PER_SCENE];
        glm::vec4 dirLightsColors[MAX_DIR_LIGHTS_PER_SCENE]; //vec4 to simplify alignments
        uint32 dirLightsCount;
    };

    struct alignas(256) ObjectConstantBufferData { //TODO: check this align value
        glm::mat4 modelMatrix;
        glm::mat4 normalMatrix; //mat4 to simplify alignments
        float parallaxOcclusionScale;
    };

    constexpr uint32 FRAME_CONSTANT_BUFFER_SIZE = sizeof(FrameConstantBufferData);
    constexpr uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData) * MAX_SCENES_PER_FRAME;
    constexpr uint32 OBJECT_CONSTANT_BUFFER_SIZE = sizeof(ObjectConstantBufferData) * MAX_OBJECTS_PER_FRAME;

    constexpr uint32 FRAME_CONSTANT_BUFFER_OFFSET = 0;
    constexpr uint32 SCENE_CONSTANT_BUFFER_OFFSET = FRAME_CONSTANT_BUFFER_SIZE;
    constexpr uint32 OBJECT_CONSTANT_BUFFER_OFFSET = FRAME_CONSTANT_BUFFER_SIZE + SCENE_CONSTANT_BUFFER_SIZE;

    static struct GraphicsData {
        Ref<CommandBuffer> commandBuffers[MAX_COMMAND_BUFFERS];
        uint32 nextCommandBufferIndex;

        Ref<Buffer> constantBuffer;

        BufferPtr frameConstantBufferPtr;
        BufferPtr sceneConstantBufferPtr;
        BufferPtr objectConstantBufferPtr;

        Ref<DescriptorSet> frameDescriptorSet;
        Ref<DescriptorSet> sceneDescriptorSet;
        Ref<DescriptorSet> objectDescriptorSet;

        //Same layout for all DescriptorSets.
        Ref<DescriptorSetLayout> defaultDescriptorSetLayout;

        GraphicsContext* graphicsContext;

        uint32 currentSceneIndex;
        uint32 currentObjectIndex;
    } data;

    Graphics::API Graphics::api = API::Unknown;


    void Graphics::init() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext = &Application::getInstance().getGraphicsContext();

        data.constantBuffer = Buffer::create(BufferType::Constant,
                                        FRAME_CONSTANT_BUFFER_SIZE + SCENE_CONSTANT_BUFFER_SIZE + OBJECT_CONSTANT_BUFFER_SIZE,
                                        MemoryType::CpuToGpu);

        data.frameConstantBufferPtr = data.constantBuffer->map(0);
        data.sceneConstantBufferPtr = data.frameConstantBufferPtr + SCENE_CONSTANT_BUFFER_OFFSET;
        data.objectConstantBufferPtr = data.frameConstantBufferPtr + OBJECT_CONSTANT_BUFFER_OFFSET;

        DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.addDescriptorDesc(DescriptorType::ConstantBufferDynamic, flagsToMask(ShaderStageFlags::All), 1);
        data.defaultDescriptorSetLayout = descriptorSetLayoutBuilder.build();

        data.frameDescriptorSet = DescriptorSet::create(data.defaultDescriptorSetLayout);
        data.frameDescriptorSet->setConstantBuffer(data.constantBuffer, 0, FRAME_CONSTANT_BUFFER_OFFSET, sizeof(FrameConstantBufferData));

        data.sceneDescriptorSet = DescriptorSet::create(data.defaultDescriptorSetLayout);
        data.sceneDescriptorSet->setConstantBuffer(data.constantBuffer, 0, SCENE_CONSTANT_BUFFER_OFFSET, sizeof(SceneConstantBufferData));

        data.objectDescriptorSet = DescriptorSet::create(data.defaultDescriptorSetLayout);
        data.objectDescriptorSet->setConstantBuffer(data.constantBuffer, 0, OBJECT_CONSTANT_BUFFER_OFFSET, sizeof(ObjectConstantBufferData));
    }

    void Graphics::destroy() {
        BZ_PROFILE_FUNCTION();

        data.constantBuffer->unmap();

        //Destroy this 'manually' to avoid the static destruction lottery
        data.constantBuffer.reset();

        data.frameDescriptorSet.reset();
        data.sceneDescriptorSet.reset();
        data.objectDescriptorSet.reset();

        data.defaultDescriptorSetLayout.reset();
    }

    uint32 Graphics::beginCommandBuffer() {
        BZ_PROFILE_FUNCTION();

        auto &commandBuffer = data.graphicsContext->getCurrentFrameCommandBuffer();
        commandBuffer->resetIndex();
        data.commandBuffers[data.nextCommandBufferIndex] = commandBuffer;

        //Auto add a BeginRenderPass Command with a force clear RenderPass on the first Command of the frame.
        auto &command = commandBuffer->addCommand(CommandType::BeginRenderPass);
        command.beginRenderPassData.framebuffer = data.graphicsContext->getCurrentFrameFramebuffer().get();
        command.beginRenderPassData.forceClearAttachments = data.nextCommandBufferIndex == 0;

        return data.nextCommandBufferIndex++;
    }

    void Graphics::endCommandBuffer(uint32 commandBufferId) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        //Auto add an EndRenderPass Command.
        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::EndRenderPass);
        commandBuffer->optimizeAndGenerate();
    }

    void Graphics::clearColorAttachments(uint32 commandBufferId, const ClearValues &clearColor) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearColorAttachments);
        command.clearAttachmentsData.framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer().get();
        command.clearAttachmentsData.clearValue = clearColor;
    }

    void Graphics::clearColorAttachments(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearColor) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearColorAttachments);
        command.clearAttachmentsData.framebuffer = framebuffer.get();
        command.clearAttachmentsData.clearValue = clearColor;
    }

    void Graphics::clearDepthStencilAttachment(uint32 commandBufferId, const ClearValues &clearValue) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearDepthStencilAttachment);
        command.clearAttachmentsData.framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer().get();
        command.clearAttachmentsData.clearValue = clearValue;
    }

    void Graphics::clearDepthStencilAttachment(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearValue) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::ClearDepthStencilAttachment);
        command.clearAttachmentsData.framebuffer = framebuffer.get();
        command.clearAttachmentsData.clearValue = clearValue;
    }

    void Graphics::beginScene(uint32 commandBufferId, const Ref<PipelineState>& pipelineState, const glm::vec3 &cameraPosition, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(data.currentSceneIndex < MAX_SCENES_PER_FRAME, "currentSceneIndex exceeded MAX_SCENES_PER_FRAME!");

        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = viewMatrix;
        sceneConstantBufferData.projectionMatrix = projectionMatrix;
        sceneConstantBufferData.viewProjectionMatrix = projectionMatrix * viewMatrix;
        sceneConstantBufferData.cameraPosition.x = cameraPosition.x;
        sceneConstantBufferData.cameraPosition.y = cameraPosition.y;
        sceneConstantBufferData.cameraPosition.z = cameraPosition.z;
        sceneConstantBufferData.dirLightsCount = 0;

        uint32 sceneOffset = data.currentSceneIndex * sizeof(SceneConstantBufferData);
        memcpy(data.sceneConstantBufferPtr + sceneOffset, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        //TODO: don't really want to bind this every frame, but ImGui shader does not use engine descriptor sets and "unbinds" them when used.
        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = data.sceneDescriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = BHAZEL_SCENE_DESCRIPTOR_SET_IDX;
        command.bindDescriptorSetData.dynamicBufferOffsets[0] = data.constantBuffer->getCurrentBaseOfReplicaOffset() + sceneOffset; //SCENE_CONSTANT_BUFFER_OFFSET is added on the static offset part 
        command.bindDescriptorSetData.dynamicBufferCount = 1;

        data.currentSceneIndex++;
    }

    void Graphics::beginScene(uint32 commandBufferId, const Ref<PipelineState>& pipelineState, const Scene &scene) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(data.currentSceneIndex < MAX_SCENES_PER_FRAME, "currentSceneIndex exceeded MAX_SCENES_PER_FRAME!");

        const Camera &camera = scene.getCamera();

        SceneConstantBufferData sceneConstantBufferData;
        sceneConstantBufferData.viewMatrix = camera.getViewMatrix();
        sceneConstantBufferData.projectionMatrix = camera.getProjectionMatrix();
        sceneConstantBufferData.viewProjectionMatrix = sceneConstantBufferData.projectionMatrix * sceneConstantBufferData.viewMatrix;
        const glm::vec3 &cameraPosition = camera.getTransform().getTranslation();
        sceneConstantBufferData.cameraPosition.x = cameraPosition.x;
        sceneConstantBufferData.cameraPosition.y = cameraPosition.y;
        sceneConstantBufferData.cameraPosition.z = cameraPosition.z;
        
        int i = 0;
        for (const auto &dirLight : scene.getDirectionalLights()) {
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].x = dirLight.direction.x;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].y = dirLight.direction.y;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].z = dirLight.direction.z;
            sceneConstantBufferData.dirLightsDirectionsAndIntensities[i].w = dirLight.intensity;
            sceneConstantBufferData.dirLightsColors[i].r = dirLight.color.r;
            sceneConstantBufferData.dirLightsColors[i].g = dirLight.color.g;
            sceneConstantBufferData.dirLightsColors[i].b = dirLight.color.b;
            i++;
        }
        sceneConstantBufferData.dirLightsCount = i;

        uint32 sceneOffset = data.currentSceneIndex * sizeof(SceneConstantBufferData);
        memcpy(data.sceneConstantBufferPtr + sceneOffset, &sceneConstantBufferData, sizeof(SceneConstantBufferData));

        //TODO: don't really want to bind this every frame, but ImGui shader does not use engine descriptor sets and "unbinds" them when used.
        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = data.sceneDescriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = BHAZEL_SCENE_DESCRIPTOR_SET_IDX;
        command.bindDescriptorSetData.dynamicBufferOffsets[0] = data.constantBuffer->getCurrentBaseOfReplicaOffset() + sceneOffset; //SCENE_CONSTANT_BUFFER_OFFSET is added on the static offset part 
        command.bindDescriptorSetData.dynamicBufferCount = 1;

        data.currentSceneIndex++;
    }

    void Graphics::beginObject(uint32 commandBufferId, const Ref<PipelineState> &pipelineState, const glm::mat4 &modelMatrix, const glm::mat3 &normalMatrix, const Material &material) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(data.currentObjectIndex < MAX_OBJECTS_PER_FRAME, "currentObjectIndex exceeded MAX_OBJECTS_PER_FRAME!");

        ObjectConstantBufferData objectConstantBufferData;
        objectConstantBufferData.modelMatrix = modelMatrix;
        objectConstantBufferData.normalMatrix = normalMatrix;
        objectConstantBufferData.parallaxOcclusionScale = material.getParallaxOcclusionScale();

        uint32 objectOffset = data.currentObjectIndex * sizeof(ObjectConstantBufferData);
        memcpy(data.objectConstantBufferPtr + objectOffset, &objectConstantBufferData, sizeof(ObjectConstantBufferData));

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = data.objectDescriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = BHAZEL_OBJECT_DESCRIPTOR_SET_IDX;
        command.bindDescriptorSetData.dynamicBufferOffsets[0] = data.constantBuffer->getCurrentBaseOfReplicaOffset() + objectOffset; //OBJECT_CONSTANT_BUFFER_OFFSET is added on the static offset part 
        command.bindDescriptorSetData.dynamicBufferCount = 1;

        data.currentObjectIndex++;
    }

    void Graphics::bindBuffer(uint32 commandBufferId, const Ref<Buffer> &buffer, uint32 offset) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(buffer->getType() == BufferType::Vertex || buffer->getType() == BufferType::Index, "Invalid Buffer type!");
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindBuffer);
        command.bindBufferData.buffer = buffer.get();
        command.bindBufferData.offset = buffer->getCurrentBaseOfReplicaOffset() + offset;
    }

    void Graphics::bindPipelineState(uint32 commandBufferId, const Ref<PipelineState> &pipelineState) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindPipelineState);
        command.bindPipelineStateData.pipelineState = pipelineState.get();
    }

    /*static void bindDescriptorSets(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet);
        graphicsContext->bindDescriptorSet(commandBuffer, descriptorSet, pipelineState);
    }*/

    void Graphics::bindDescriptorSet(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet, 
                                     const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                     uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(dynamicBufferCount < MAX_DESCRIPTOR_DYNAMIC_OFFSETS, "Invalid dynamicBufferCount: {}!", dynamicBufferCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        //Mix correctly the dynamicBufferOffsets coming from the user with the ones that the engine needs to send behind the scenes for dynamic buffers.
        uint32 finalDynamicBufferOffsets[MAX_DESCRIPTOR_DYNAMIC_OFFSETS];
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

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = descriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = setIndex;
        memcpy(command.bindDescriptorSetData.dynamicBufferOffsets, finalDynamicBufferOffsets, dynIndex * sizeof(uint32));
        command.bindDescriptorSetData.dynamicBufferCount = dynamicBufferCount;
    }

    void Graphics::setPushConstants(uint32 commandBufferId, const Ref<PipelineState> &pipelineState, uint8 shaderStageMask, const void *data, uint32 size, uint32 offset) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        BZ_ASSERT_CORE(size % 4 == 0, "Size must be a multiple of 4!");
        BZ_ASSERT_CORE(offset % 4 == 0, "Offset must be a multiple of 4!");
        BZ_ASSERT_CORE(size <= MAX_PUSH_CONSTANT_SIZE, "Push constant size must be less or equal than {}. Sending size: {}!", MAX_PUSH_CONSTANT_SIZE, size);

        auto& commandBuffer = BZ::data.commandBuffers[commandBufferId];
        auto& command = commandBuffer->addCommand(CommandType::SetPushConstants);
        command.setPushConstantsData.pipelineState = pipelineState.get();
        command.setPushConstantsData.shaderStageMask = shaderStageMask;
        memcpy(&command.setPushConstantsData.data, data, size);
        command.setPushConstantsData.size = size;
        command.setPushConstantsData.offset = offset;
    }

    void Graphics::draw(uint32 commandBufferId, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::Draw);
        command.drawData.vertexCount = vertexCount;
        command.drawData.instanceCount = instanceCount;
        command.drawData.firstVertex = firstVertex;
        command.drawData.firstInstance = firstInstance;
    }

    void Graphics::drawIndexed(uint32 commandBufferId, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::DrawIndexed);
        command.drawIndexedData.indexCount = indexCount;
        command.drawIndexedData.instanceCount = instanceCount;
        command.drawIndexedData.firstIndex = firstIndex;
        command.drawIndexedData.vertexOffset = vertexOffset;
        command.drawIndexedData.firstInstance = firstInstance;
    }

    void Graphics::setViewports(uint32 commandBufferId, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(viewportCount < MAX_VIEWPORTS, "Invalid viewportCount: {}!", viewportCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::SetViewports);
        command.setViewportsData.firstIndex = firstIndex;
        memcpy(command.setViewportsData.viewports, viewports, viewportCount * sizeof(Viewport));
        command.setViewportsData.viewportCount = viewportCount;
    }

    void Graphics::setScissorRects(uint32 commandBufferId, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(rectCount < MAX_VIEWPORTS, "Invalid rectCount: {}!", rectCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::SetScissorRects);
        command.setScissorRectsData.firstIndex = firstIndex;
        memcpy(command.setScissorRectsData.rects, rects, rectCount * sizeof(ScissorRect));
        command.setScissorRectsData.rectCount = rectCount;
    }

    void Graphics::waitForDevice() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext->waitForDevice();
    }

    Ref<DescriptorSetLayout>& Graphics::getDefaultDescriptorSetLayout() {
        return data.defaultDescriptorSetLayout;
    }

    void Graphics::beginFrame() {
        BZ_PROFILE_FUNCTION();

        data.nextCommandBufferIndex = 0;
        data.currentSceneIndex = 0;
        data.currentObjectIndex = 0;
    }

    void Graphics::endFrame() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext->submitCommandBuffersAndFlush(data.commandBuffers, data.nextCommandBufferIndex);
        //No need to delete CommandBuffers. The pools are responsible for that.
    }

    void Graphics::onWindowResize(const WindowResizedEvent &ev) {
        //TODO: do something to viewport
    }
}