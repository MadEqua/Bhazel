#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/Internal/Queue.h"

#include "Graphics/GpuObject.h"


namespace BZ {

    class Framebuffer;
    class RenderPass;
    union ClearValues;
    class Texture;
    class Buffer;
    class DescriptorSet;
    class PipelineState;
    class PipelineLayout;

    /*
    * CommandPools create the CommandBuffers.
    */
    class CommandBuffer : public GpuObject<VkCommandBuffer> {
    public:
        static CommandBuffer& getAndBegin(QueueProperty queueProperty);

        BZ_NON_COPYABLE(CommandBuffer);

        void begin();

        void end();
        void endAndSubmit();
        void endAndSubmitImmediately();

        void submit();
        void submitImmediately();

        void beginRenderPass(const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer);
        void endRenderPass();

        void nextSubPass();

        void clearColorAttachments(const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor);
        void clearDepthStencilAttachment(const Ref<Framebuffer>& framebuffer, const VkClearDepthStencilValue &clearValue) ;

        void bindBuffer(const Ref<Buffer> &buffer, uint32 offset);

        void bindPipelineState(const Ref<PipelineState> &pipelineState);
        //void bindDescriptorSets(const Ref<DescriptorSet> &descriptorSet);
        void bindDescriptorSet(const DescriptorSet &descriptorSet,
                               const Ref<PipelineLayout> &pipelineLayout, uint32 setIndex,
                               uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

        void setPushConstants(const Ref<PipelineLayout> &pipelineLayout, VkShaderStageFlags shaderStageFlags,
                              const void* data, uint32 size, uint32 offset);

        void draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        void drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        //Pipeline dynamic state changes
        void setViewports(uint32 firstIndex, const VkViewport viewports[], uint32 viewportCount);
        void setScissorRects(uint32 firstIndex, const VkRect2D rects[], uint32 rectCount);
        void setDepthBias(float constantFactor, float clamp, float slopeFactor);

        //Sync and Transitions
        void pipelineBarrierMemory(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                                   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

        void pipelineBarrierTexture(const Ref<Texture> &texture,
                                    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                                    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                    VkImageLayout oldLayout, VkImageLayout newLayout,
                                    uint32 baseMipLevel, uint32 mipLevels);
        void pipelineBarrierTexture(const Texture &texture,
                                    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                                    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                    VkImageLayout oldLayout, VkImageLayout newLayout,
                                    uint32 baseMipLevel, uint32 mipLevels);

        //Transfer
        void copyBufferToBuffer(const Ref<Buffer> &src, const Ref<Buffer> &dst, VkBufferCopy regions[], uint32 regionCount);
        void copyBufferToBuffer(const Buffer &src, const Buffer &dst, VkBufferCopy regions[], uint32 regionCount);

        void copyBufferToTexture(const Ref<Buffer> &src, const Ref<Texture> &dst, VkImageLayout imageLayout, const VkBufferImageCopy regions[], uint32 regionCount);
        void copyBufferToTexture(const Buffer &src, const Texture &dst, VkImageLayout imageLayout, const VkBufferImageCopy regions[], uint32 regionCount);

        void blitTexture(const Ref<Texture> &src, const Ref<Texture> &dst, VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageBlit blit[], uint32 blitCount, VkFilter filter);
        void blitTexture(const Texture &src, const Texture &dst, VkImageLayout srcLayout, VkImageLayout dstLayout, VkImageBlit blit[], uint32 blitCount, VkFilter filter);

        uint32 getCommandCount() const { return commandCount; }
        uint32 getQueueFamilyIndex() const { return queueFamilyIndex; }

    private:
        uint32 commandCount;

        //The queue family which this Buffer will be submitted to.
        uint32 queueFamilyIndex;

        CommandBuffer() = default;
        void init(VkCommandBuffer vkCommandBuffer, uint32 queueFamilyIndex);

        friend class CommandPool;
    };
}
