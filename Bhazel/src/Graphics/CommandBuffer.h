#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/Internal/Queue.h"

#include "Graphics/GpuObject.h"


namespace BZ {

    class Framebuffer;
    union ClearValues;
    class Texture;
    class Buffer;
    class DescriptorSet;
    class PipelineState;

    /*
    * CommandPools create the CommandBuffers.
    */
    class CommandBuffer : public GpuObject<VkCommandBuffer> {
    public:
        static CommandBuffer& getAndBegin(QueueProperty queueProperty, bool exclusiveQueue = false);

        BZ_NON_COPYABLE(CommandBuffer);

        void begin();

        void end();
        void endAndSubmit();
        void endAndSubmitImmediately();

        void submit();
        void submitImmediately();

        void beginRenderPass(const Ref<Framebuffer> &framebuffer, bool forceClearAttachments);
        void endRenderPass();

        void nextSubPass();

        void clearColorAttachments(const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor);
        void clearDepthStencilAttachment(const Ref<Framebuffer>& framebuffer, const VkClearDepthStencilValue &clearValue) ;

        void bindBuffer(const Ref<Buffer> &buffer, uint32 offset);

        void bindPipelineState(const Ref<PipelineState> &pipelineState);
        //void bindDescriptorSets(const Ref<DescriptorSet> &descriptorSet);
        void bindDescriptorSet(const DescriptorSet &descriptorSet,
                               const Ref<PipelineState> &pipelineState, uint32 setIndex,
                               uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

        void setPushConstants(const Ref<PipelineState> &pipelineState, VkShaderStageFlags shaderStageFlags,
                              const void* data, uint32 size, uint32 offset);

        void draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        void drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        //Pipeline dynamic state changes
        void setViewports(uint32 firstIndex, const VkViewport viewports[], uint32 viewportCount);
        void setScissorRects(uint32 firstIndex, const VkRect2D rects[], uint32 rectCount);
        void setDepthBias(float constantFactor, float clamp, float slopeFactor);

        //Sync
        void pipelineBarrierTexture(const Ref<Texture> &texture);

        //Transfer
        void copyBuffer(const Ref<Buffer> &src, const Ref<Buffer> &dst, VkBufferCopy regions[], uint32 regionCount);
        void copyBuffer(const Buffer &src, const Buffer &dst, VkBufferCopy regions[], uint32 regionCount);

        uint32 getCommandCount() const { return commandCount; }
        QueueProperty getQueueProperty() const { return queueProperty; }
        bool isExclusiveQueue() const { return exclusiveQueue; }

    private:
        uint32 commandCount;

        //The queue which this Buffer will be submitted.
        QueueProperty queueProperty;
        bool exclusiveQueue;

        CommandBuffer() = default;
        void init(VkCommandBuffer vkCommandBuffer, QueueProperty queueProperty, bool exclusiveQueue);

        friend class CommandPool;
    };
}
