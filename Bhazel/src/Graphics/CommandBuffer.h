#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    class Framebuffer;
    union ClearValues;
    class Texture;
    class Buffer;
    class DescriptorSet;

    /*
    * CommandPools create the CommandBuffers.
    */
    class CommandBuffer : public GpuObject<VkCommandBuffer> {
    public:
        void begin();
        void end();

        void beginRenderPass(const Ref<Framebuffer> &framebuffer, bool forceClearAttachments);
        void endRenderPass();

        void nextSubPass();

        void clearColorAttachments(const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor);
        void clearDepthStencilAttachment(const Ref<Framebuffer>& framebuffer, const VkClearDepthStencilValue &clearValue) ;

        void bindBuffer(const Ref<Buffer> &buffer, uint32 offset);

        void bindPipelineState(const Ref<PipelineState> &pipelineState);
        //void bindDescriptorSets(const Ref<DescriptorSet> &descriptorSet);
        void bindDescriptorSet(const Ref<DescriptorSet> &descriptorSet,
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

        uint32 getCommandCount() const { return commandCount; }

    private:
        uint32 commandCount;

        static Ref<CommandBuffer> wrap(VkCommandBuffer vkCommandBuffer);
        explicit CommandBuffer(VkCommandBuffer vkCommandBuffer);

        friend class CommandPool;
    };
}
