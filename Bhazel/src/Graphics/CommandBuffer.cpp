#include "bzpch.h"

#include "CommandBuffer.h"

#include "Graphics/DescriptorSet.h"


namespace BZ {

    Command& CommandBuffer::addCommand(CommandType type) {
        BZ_ASSERT_CORE(nextCommandIndex < MAX_COMMANDS_PER_BUFFER, "Invalid nextCommandIndex: {}!", nextCommandIndex);
        Command &cmd = commands[nextCommandIndex++];
        cmd.type = type;
        return cmd;
    }

    void CommandBuffer::optimizeAndGenerate() {
        //TODO: optimize commands

        begin();

        for(uint32 cmdIdx = 0; cmdIdx < nextCommandIndex; ++cmdIdx) {
            Command &cmd = commands[cmdIdx];

            switch(cmd.type) {
            case CommandType::BeginRenderPass:
                beginRenderPass(*cmd.beginRenderPassData.framebuffer, cmd.beginRenderPassData.forceClearAttachments);
                break;
            case CommandType::EndRenderPass:
                endRenderPass();
                break;
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
            case CommandType::SetPushConstants:
                setPushConstants(*cmd.setPushConstantsData.pipelineState, cmd.setPushConstantsData.shaderStageMask, cmd.setPushConstantsData.data, cmd.setPushConstantsData.size, cmd.setPushConstantsData.offset);
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
            case CommandType::SetDepthBias:
                setDepthBias(cmd.setDepthBiasData.constantFactor, cmd.setDepthBiasData.clamp, cmd.setDepthBiasData.slopeFactor);
                break;
            case CommandType::PipelineBarrierTexture:
                pipelineBarrierTexture(*cmd.pipelineBarrierTexture.texture);
                break;
            default:
                BZ_ASSERT_ALWAYS("Invalid Command Type!");
                break;
            }
        }

        end();
    }
}