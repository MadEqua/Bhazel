#include "bzpch.h"

#include "Graphics.h"

#include "Core/Application.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Buffer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Shader.h"
#include "Graphics/Color.h"
#include "Graphics/CommandBuffer.h"


namespace BZ {  

    static struct GraphicsData {
        Ref<CommandBuffer> commandBuffers[MAX_COMMAND_BUFFERS];
        uint32 nextCommandBufferIndex;

        GraphicsContext* graphicsContext;
    } data;

    Graphics::API Graphics::api = API::Unknown;


    void Graphics::init() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext = &Application::getInstance().getGraphicsContext();  
    }

    void Graphics::destroy() {
        BZ_PROFILE_FUNCTION();
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

        BZ_ASSERT_CORE(dynamicBufferCount <= MAX_DESCRIPTOR_DYNAMIC_OFFSETS && dynamicBufferCount <= descriptorSet->getDynamicBufferCount(),
            "Invalid dynamicBufferCount: {}!", dynamicBufferCount);
        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        //Mix correctly the dynamicBufferOffsets coming from the user with the ones that the engine needs to send behind the scenes for dynamic buffers.
        uint32 finalDynamicBufferOffsets[MAX_DESCRIPTOR_DYNAMIC_OFFSETS];
        uint32 index = 0;
        uint32 userIndex = 0;
        uint32 binding = 0;
        for(const auto &desc : descriptorSet->getLayout()->getDescriptorDescs()) {
            if(desc.type == DescriptorType::ConstantBufferDynamic || desc.type == DescriptorType::StorageBufferDynamic) {
                const auto *dynBufferData = descriptorSet->getDynamicBufferDataByBinding(binding);
                BZ_ASSERT_CORE(dynBufferData, "Non-existent binding should not happen!");
                finalDynamicBufferOffsets[index] = dynBufferData->buffer->getCurrentBaseOfReplicaOffset();
                if(userIndex < dynamicBufferCount) {
                    finalDynamicBufferOffsets[index] += dynamicBufferOffsets[userIndex++];
                }
                else {

                }
                index++;
            }
            binding++;
        }

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindDescriptorSet);
        command.bindDescriptorSetData.descriptorSet = descriptorSet.get();
        command.bindDescriptorSetData.pipelineState = pipelineState.get();
        command.bindDescriptorSetData.setIndex = setIndex;
        memcpy(command.bindDescriptorSetData.dynamicBufferOffsets, finalDynamicBufferOffsets, index * sizeof(uint32));
        command.bindDescriptorSetData.dynamicBufferCount = index;
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

    void Graphics::beginFrame() {
        BZ_PROFILE_FUNCTION();

        data.nextCommandBufferIndex = 0;
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