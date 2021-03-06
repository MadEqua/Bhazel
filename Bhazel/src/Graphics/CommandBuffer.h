#pragma once

#include "Graphics/Internal/Queue.h"
#include "Graphics/Internal/VulkanIncludes.h"

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
class QueryPool;

/*
 * CommandPools create the CommandBuffers.
 */
class CommandBuffer : public GpuObject<VkCommandBuffer> {
  public:
    static CommandBuffer &getAndBegin(QueueProperty queueProperty);

    void begin();
    void end();
    void endAndSubmit(bool waitForImageAvailable = false, bool signalFrameEnd = false);

    void submit(bool waitForImageAvailable = false, bool signalFrameEnd = false);

    void beginRenderPass(const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer);
    void endRenderPass();

    void nextSubPass();

    void clearColorAttachments(const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor);
    void clearDepthStencilAttachment(const Ref<Framebuffer> &framebuffer, const VkClearDepthStencilValue &clearValue);

    void bindBuffer(const Ref<Buffer> &buffer, uint32 offset);

    void bindPipelineState(const Ref<PipelineState> &pipelineState);
    // void bindDescriptorSets(const Ref<DescriptorSet> &descriptorSet);
    void bindDescriptorSet(const DescriptorSet &descriptorSet, const Ref<PipelineLayout> &pipelineLayout,
                           uint32 setIndex, uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

    void setPushConstants(const Ref<PipelineLayout> &pipelineLayout, VkShaderStageFlags shaderStageFlags,
                          const void *data, uint32 offset, uint32 size);

    void draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
    void drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset,
                     uint32 firstInstance);

    // Pipeline dynamic state changes
    void setViewports(uint32 firstIndex, const VkViewport viewports[], uint32 viewportCount);
    void setScissorRects(uint32 firstIndex, const VkRect2D rects[], uint32 rectCount);
    void setDepthBias(float constantFactor, float clamp, float slopeFactor);

    // Sync and Transitions
    void pipelineBarrierMemory(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                               VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

    void pipelineBarrierTexture(const Ref<Texture> &texture, VkPipelineStageFlags srcStage,
                                VkPipelineStageFlags dstStage, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                VkImageLayout oldLayout, VkImageLayout newLayout, uint32 baseMipLevel,
                                uint32 mipLevels);
    void pipelineBarrierTexture(const Texture &texture, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                                VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                VkImageLayout newLayout, uint32 baseMipLevel, uint32 mipLevels);

    // Transfer
    void copyBufferToBuffer(const Ref<Buffer> &src, const Ref<Buffer> &dst, VkBufferCopy regions[], uint32 regionCount);
    void copyBufferToBuffer(const Buffer &src, const Buffer &dst, VkBufferCopy regions[], uint32 regionCount);

    void copyBufferToTexture(const Ref<Buffer> &src, const Ref<Texture> &dst, VkImageLayout imageLayout,
                             const VkBufferImageCopy regions[], uint32 regionCount);
    void copyBufferToTexture(const Buffer &src, const Texture &dst, VkImageLayout imageLayout,
                             const VkBufferImageCopy regions[], uint32 regionCount);

    void blitTexture(const Ref<Texture> &src, const Ref<Texture> &dst, VkImageLayout srcLayout, VkImageLayout dstLayout,
                     VkImageBlit blit[], uint32 blitCount, VkFilter filter);
    void blitTexture(const Texture &src, const Texture &dst, VkImageLayout srcLayout, VkImageLayout dstLayout,
                     VkImageBlit blit[], uint32 blitCount, VkFilter filter);

    // Queries
    void resetQueryPool(const QueryPool &pool, uint32 firstQuery, uint32 queryCount);
    void beginQuery(const QueryPool &pool, uint32 queryIndex, VkQueryControlFlags controlFlags);
    void endQuery(const QueryPool &pool, uint32 queryIndex);
    void writeTimestamp(VkPipelineStageFlagBits pipelineStage, const QueryPool &pool, uint32 queryIndex);
    void copyQueryPoolResults(const QueryPool &pool, uint32 firstQuery, uint32 queryCount, const Ref<Buffer> &dstBuffer,
                              uint32 dstOffset, uint32 stride, VkQueryResultFlags flags);

    // Debug
#ifdef BZ_GRAPHICS_DEBUG
    void beginDebugLabel(const char *label, const glm::vec4 &color = glm::vec4(1.0f));
    void endDebugLabel();
    void insertDebugLabel(const char *label, const glm::vec4 &color = glm::vec4(1.0f));
#endif

    // Statistics
    uint32 getCommandCount() const { return commandCount; }
    uint32 getQueueFamilyIndex() const { return queueFamilyIndex; }

  private:
    uint32 commandCount;

    // The queue family which this Buffer will be submitted to.
    uint32 queueFamilyIndex;

    void init(VkCommandBuffer vkCommandBuffer, uint32 queueFamilyIndex);
    friend class CommandPool;
};

#ifdef BZ_GRAPHICS_DEBUG
    #define BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, ...) commandBuffer.beginDebugLabel(__VA_ARGS__)
    #define BZ_CB_END_DEBUG_LABEL(commandBuffer) commandBuffer.endDebugLabel()
    #define BZ_CB_INSERT_DEBUG_LABEL(commandBuffer, ...) commandBuffer.insertDebugLabel(__VA_ARGS__)
#else
    #define BZ_CB_BEGIN_DEBUG_LABEL(commandBuffer, ...)
    #define BZ_CB_END_DEBUG_LABEL(commandBuffer)
    #define BZ_CB_INSERT_DEBUG_LABEL(commandBuffer, ...)
#endif
}
