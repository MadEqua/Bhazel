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
                bindBuffer(*cmd.bindBufferData.buffer, cmd.bindBufferData.buffer->getCurrentBaseOfReplicaOffset() + cmd.bindBufferData.offset);
                break;
            case CommandType::BindPipelineState:
                bindPipelineState(*cmd.bindPipelineStateData.pipelineState);
                break;
            case CommandType::BindDescriptorSet: {
                //Mix correctly the dynamicBufferOffsets coming from the user with the ones that the engine needs to send behind the scenes for dynamic buffers.
                uint32 finalDynamicBufferOffsets[MAX_DESCRIPTOR_DYNAMIC_OFFSETS + 3];
                uint32 dynIndex = 0;
                uint32 userDynIndex = 0;
                uint32 binding = 0;
                for(const auto &desc : cmd.bindDescriptorSetData.descriptorSet->getLayout()->getDescriptorDescs()) {
                    if(desc.type == DescriptorType::ConstantBufferDynamic || desc.type == DescriptorType::StorageBufferDynamic) {
                        const auto *dynBufferData = cmd.bindDescriptorSetData.descriptorSet->getDynamicBufferDataByBinding(binding);
                        BZ_ASSERT_CORE(dynBufferData, "Non-existent binding should not happen!");
                        finalDynamicBufferOffsets[dynIndex] = dynBufferData->buffer->getCurrentBaseOfReplicaOffset();
                        if(!dynBufferData->isAutoAddedByEngine) {
                            finalDynamicBufferOffsets[dynIndex] += cmd.bindDescriptorSetData.dynamicBufferOffsets[userDynIndex++];
                        }
                        dynIndex++;
                    }
                    binding++;
                }

                bindDescriptorSet(*cmd.bindDescriptorSetData.descriptorSet, *cmd.bindDescriptorSetData.pipelineState, cmd.bindDescriptorSetData.setIndex, finalDynamicBufferOffsets, dynIndex);
            }
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