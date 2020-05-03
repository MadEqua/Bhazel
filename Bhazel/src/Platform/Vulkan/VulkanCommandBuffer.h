#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"
#include "Graphics/CommandBuffer.h"


namespace BZ {

    class Framebuffer;
    class Texture;

    class VulkanCommandBuffer : public CommandBuffer, public VulkanGpuObject<VkCommandBuffer> {
    public:
        static Ref<VulkanCommandBuffer> wrap(VkCommandBuffer vkCommandBuffer);

        explicit VulkanCommandBuffer(VkCommandBuffer vkCommandBuffer);

    protected:
        void begin() override;
        void end() override;

        void beginRenderPass(const Framebuffer &framebuffer, bool forceClearAttachments) override;
        void endRenderPass() override;

        void nextSubPass() override;

        void clearColorAttachments(const Framebuffer &framebuffer, const ClearValues &clearColor) override;
        void clearDepthStencilAttachment(const Framebuffer &framebuffer, const ClearValues &clearValue) override;

        void bindBuffer(const Buffer &buffer, uint32 offset) override;

        void bindPipelineState(const PipelineState &pipelineState) override;
        //void bindDescriptorSets(const DescriptorSet&descriptorSet) override;
        void bindDescriptorSet(const DescriptorSet &descriptorSet,
                               const PipelineState &pipelineState, uint32 setIndex,
                               uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) override;

        void setPushConstants(const PipelineState &pipelineState, uint8 shaderStageMask,
            const void* data, uint32 size, uint32 offset) override;

        void draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) override;
        void drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;

        //Pipeline dynamic state changes
        void setViewports(uint32 firstIndex, const Command::Viewport viewports[], uint32 viewportCount) override;
        void setScissorRects(uint32 firstIndex, const Command::ScissorRect rects[], uint32 rectCount) override;
        void setDepthBias(float constantFactor, float clamp, float slopeFactor) override;

        //Sync
        void pipelineBarrierTexture(const Texture &texture);
    };
}