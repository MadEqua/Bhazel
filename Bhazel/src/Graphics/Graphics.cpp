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

#include <imgui.h>


namespace BZ {

    constexpr static int FRAME_HISTORY_SIZE = 100;

    struct GraphicsStats {
        FrameStats frameStats;
        uint32 commandBufferCount;
        uint32 commandCount;
        uint32 renderPassCount;
        uint32 drawCallCount;
        uint32 bufferBindCount;
        uint32 descriptorSetBindCount;
        uint32 pipelineStateBindCount;
    };

    static struct GraphicsData {
        Ref<CommandBuffer> commandBuffers[MAX_COMMAND_BUFFERS];
        uint32 nextCommandBufferIndex;

        GraphicsContext* graphicsContext;

        std::set<Ref<Framebuffer>> clearedFramebuffers;

        //Stats
        GraphicsStats stats;
        GraphicsStats visibleStats;

        uint64 statsRefreshPeriodMs = 250;
        uint64 statsTimeAcumMs;

        float frameTimeHistory[FRAME_HISTORY_SIZE] = {};
        uint32 frameTimeHistoryIdx;
    } data;

    Graphics::API Graphics::api = API::Unknown;


    void Graphics::init() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext = &Application::getInstance().getGraphicsContext();  
    }

    void Graphics::destroy() {
        BZ_PROFILE_FUNCTION();

        data.clearedFramebuffers.clear();
    }

    uint32 Graphics::beginCommandBuffer() {
        BZ_PROFILE_FUNCTION();

        auto &commandBuffer = data.graphicsContext->getCurrentFrameCommandBuffer();
        commandBuffer->resetIndex();
        data.commandBuffers[data.nextCommandBufferIndex] = commandBuffer;
        return data.nextCommandBufferIndex++;
    }

    void Graphics::endCommandBuffer(uint32 commandBufferId) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);
        data.commandBuffers[commandBufferId]->optimizeAndGenerate();

        data.stats.commandBufferCount++;
        data.stats.commandCount += data.commandBuffers[commandBufferId]->getCommandCount();
    }

    void Graphics::beginRenderPass(uint32 commandBufferId) {
        BZ_PROFILE_FUNCTION();

        beginRenderPass(commandBufferId, data.graphicsContext->getCurrentFrameFramebuffer());
    }

    void Graphics::beginRenderPass(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BeginRenderPass);
        command.beginRenderPassData.framebuffer = framebuffer.get();
        command.beginRenderPassData.forceClearAttachments = data.clearedFramebuffers.find(framebuffer) == data.clearedFramebuffers.end();

        data.clearedFramebuffers.emplace(framebuffer);
    }

    void Graphics::endRenderPass(uint32 commandBufferId) {
        BZ_PROFILE_FUNCTION();

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        commandBuffer->addCommand(CommandType::EndRenderPass);

        data.stats.renderPassCount++;
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

        data.stats.bufferBindCount++;
    }

    void Graphics::bindPipelineState(uint32 commandBufferId, const Ref<PipelineState> &pipelineState) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::BindPipelineState);
        command.bindPipelineStateData.pipelineState = pipelineState.get();

        data.stats.pipelineStateBindCount++;
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
                else if(dynamicBufferCount > 0) {
                    BZ_LOG_CORE_WARN("Graphics::bindDescriptorSet(): there are more dynamic buffers on the DescriptorSet than the number of offsets sent to the bind function. Might be an error.");
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

        data.stats.descriptorSetBindCount++;
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

        data.stats.drawCallCount++;
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

        data.stats.drawCallCount++;
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

    void Graphics::setDepthBias(uint32 commandBufferId, float constantFactor, float clamp, float slopeFactor) {
        BZ_PROFILE_FUNCTION();

        BZ_ASSERT_CORE(commandBufferId < MAX_COMMAND_BUFFERS, "Invalid commandBufferId: {}!", commandBufferId);

        auto &commandBuffer = data.commandBuffers[commandBufferId];
        auto &command = commandBuffer->addCommand(CommandType::SetDepthBias);
        command.setDepthBiasData.constantFactor = constantFactor;
        command.setDepthBiasData.clamp = clamp;
        command.setDepthBiasData.slopeFactor = slopeFactor;
    }

    void Graphics::waitForDevice() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext->waitForDevice();
    }

    void Graphics::beginFrame() {
        BZ_PROFILE_FUNCTION();

        data.nextCommandBufferIndex = 0;
        data.clearedFramebuffers.clear();

        memset(&data.stats, 0, sizeof(GraphicsStats));
    }

    void Graphics::endFrame() {
        BZ_PROFILE_FUNCTION();

        data.graphicsContext->submitCommandBuffersAndFlush(data.commandBuffers, data.nextCommandBufferIndex);
        //No need to delete CommandBuffers. The pools are responsible for that.
    }

    void Graphics::onImGuiRender(const FrameStats &frameStats) {
        BZ_PROFILE_FUNCTION();

        data.stats.frameStats = frameStats;

        data.statsTimeAcumMs += frameStats.lastFrameTime.asMillisecondsUint64();
        if (data.statsTimeAcumMs >= data.statsRefreshPeriodMs) {
            data.statsTimeAcumMs = 0;
            data.visibleStats = data.stats;
            data.frameTimeHistory[data.frameTimeHistoryIdx] = frameStats.lastFrameTime.asMillisecondsFloat();
            data.frameTimeHistoryIdx = (data.frameTimeHistoryIdx + 1) % FRAME_HISTORY_SIZE;
        }

        if (ImGui::Begin("Graphics")) {
            ImGui::Text("FrameStats:");
            ImGui::Text("Last Frame Time: %.3f ms.", data.visibleStats.frameStats.lastFrameTime.asMillisecondsFloat());
            ImGui::Text("FPS: %.3f.", 1.0f / data.visibleStats.frameStats.lastFrameTime.asSeconds());
            //ImGui::Separator();
            ImGui::Text("Avg Frame Time: %.3f ms.", data.visibleStats.frameStats.runningTime.asMillisecondsFloat() / static_cast<float>(data.visibleStats.frameStats.frameCount));
            ImGui::Text("Avg FPS: %.3f.", static_cast<float>(data.visibleStats.frameStats.frameCount) / data.visibleStats.frameStats.runningTime.asSeconds());
            //ImGui::Separator();
            ImGui::Text("Frame Count: %d.", data.visibleStats.frameStats.frameCount);
            ImGui::Text("Running Time: %.3f seconds.", data.visibleStats.frameStats.runningTime.asSeconds());
            ImGui::Separator();

            ImGui::Text("Stats:");
            ImGui::Text("CommandBuffer Count: %d", data.visibleStats.commandBufferCount);
            ImGui::Text("Command Count: %d", data.visibleStats.commandCount);
            ImGui::Text("Render Pass Count: %d", data.visibleStats.renderPassCount);
            ImGui::Text("Draw Call Count: %d", data.visibleStats.drawCallCount);
            ImGui::Text("Buffer Bind Count: %d", data.visibleStats.bufferBindCount);
            ImGui::Text("DescriptorSet Bind Count: %d", data.visibleStats.descriptorSetBindCount);
            ImGui::Text("PipelineState Bind Count: %d", data.visibleStats.pipelineStateBindCount);
            ImGui::Separator();

            ImGui::PlotLines("Frame Times", data.frameTimeHistory, FRAME_HISTORY_SIZE, data.frameTimeHistoryIdx, "ms", 0.0f, 50.0f, ImVec2(0, 80));
            ImGui::Separator();

            ImGui::SliderInt("Refresh period ms", reinterpret_cast<int*>(&data.statsRefreshPeriodMs), 0, 1000);
        }
        ImGui::End();
    }

    void Graphics::onWindowResize(const WindowResizedEvent &ev) {
        //TODO: do something to viewport
    }
}