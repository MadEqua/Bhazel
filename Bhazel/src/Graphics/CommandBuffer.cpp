#include "bzpch.h"

#include "CommandBuffer.h"

#include "Graphics/DescriptorSet.h"


namespace BZ {

    Command& CommandBuffer::addCommand(CommandType type) {
        BZ_ASSERT_CORE(currentIndex < MAX_COMMANDS_PER_BUFFER, "Invalid currentIndex: {}!", currentIndex);
        Command &cmd = commands[currentIndex++];
        cmd.type = type;
        return cmd;
    }

    void CommandBuffer::optimizeAndGenerate(const Ref<Framebuffer> &framebuffer) {
        //TODO: optimize commands

        begin(framebuffer);

        for(uint32 cmdIdx = 0; cmdIdx < currentIndex; ++cmdIdx) {
            Command &cmd = commands[cmdIdx];

            switch(cmd.type) {
            case CommandType::ClearColorAttachments:
                clearColorAttachments(*cmd.clearAttachmentsData.framebuffer, cmd.clearAttachmentsData.clearValue);
                break;
            case CommandType::ClearDepthStencilAttachment:
                clearDepthStencilAttachment(*cmd.clearAttachmentsData.framebuffer, cmd.clearAttachmentsData.clearValue);
                break;
            case CommandType::BindBuffer:
                bindBuffer(*cmd.bindBufferData.buffer, cmd.bindBufferData.offset);
                break;
            case CommandType::BindPipelineState:
                bindPipelineState(*cmd.bindPipelineStateData.pipelineState);
                break;
            case CommandType::BindDescriptorSet:
                bindDescriptorSet(*cmd.bindDescriptorSetData.descriptorSet, *cmd.bindDescriptorSetData.pipelineState, cmd.bindDescriptorSetData.setIndex, cmd.bindDescriptorSetData.dynamicBufferOffsets, cmd.bindDescriptorSetData.dynamicBufferCount);
                break;
            case CommandType::Draw:
                draw(cmd.drawData.vertexCount, cmd.drawData.instanceCount, cmd.drawData.firstVertex, cmd.drawData.firstInstance);
                break;
            case CommandType::DrawIndexed:
                drawIndexed(cmd.drawIndexedData.indexCount, cmd.drawIndexedData.instanceCount, cmd.drawIndexedData.firstIndex, cmd.drawIndexedData.vertexOffset, cmd.drawIndexedData.firstInstance);
                break;
            case CommandType::SetViewports:
                setViewports(cmd.setViewportsData.firstIndex, cmd.setViewportsData.viewports, cmd.setViewportsData.viewportCount);
                break;
            case CommandType::SetScissorRects:
                setScissorRects(cmd.setScissorRectsData.firstIndex, cmd.setScissorRectsData.rects, cmd.setScissorRectsData.rectCount);
                break;
            default:
                BZ_ASSERT_ALWAYS("Invalid Command Type!");
                break;
            }
        }

        end();
    }
}