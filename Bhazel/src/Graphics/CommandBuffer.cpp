#include "bzpch.h"

#include "CommandBuffer.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"


namespace BZ {

    Ref<CommandBuffer> CommandBuffer::wrap(VkCommandBuffer vkCommandBuffer) {
        return MakeRef<CommandBuffer>(new CommandBuffer(vkCommandBuffer));
    }

    CommandBuffer::CommandBuffer(VkCommandBuffer vkCommandBuffer) {
        handle = vkCommandBuffer;
    }

    void CommandBuffer::begin() {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //Disallowing command buffer reusage
        beginInfo.pInheritanceInfo = nullptr;

        BZ_ASSERT_VK(vkBeginCommandBuffer(handle, &beginInfo));

        commandCount = 0;
    }

    void CommandBuffer::end() {
        BZ_ASSERT_VK(vkEndCommandBuffer(handle));
    }

    void CommandBuffer::beginRenderPass(const Ref<Framebuffer> &framebuffer, bool forceClearAttachments) {
        auto &renderPass = framebuffer->getRenderPass();

        //We know that the color attachments will be first and then the depthstencil
        VkClearValue clearValues[MAX_FRAMEBUFFER_ATTACHEMENTS];
        uint32 i;
        for (i = 0; i < renderPass->getColorAttachmentCount(); ++i) {
            const auto &attDesc = renderPass->getColorAttachmentDescription(i);
            if (attDesc.loadOperatorColorAndDepth == VK_ATTACHMENT_LOAD_OP_CLEAR || forceClearAttachments) {
                clearValues[i].color = attDesc.clearValue.color;
            }
        }
        if (renderPass->hasDepthStencilAttachment()) {
            const auto attDesc = renderPass->getDepthStencilAttachmentDescription();
            if (attDesc->loadOperatorStencil == VK_ATTACHMENT_LOAD_OP_CLEAR || forceClearAttachments) {
                clearValues[i].depthStencil = attDesc->clearValue.depthStencil;
            }
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = forceClearAttachments ? renderPass->getHandle().forceClear : renderPass->getHandle().original;
        renderPassBeginInfo.framebuffer = framebuffer->getHandle();
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(framebuffer->getDimensionsAndLayers().x), static_cast<uint32_t>(framebuffer->getDimensionsAndLayers().y) };
        renderPassBeginInfo.clearValueCount = renderPass->getAttachmentCount();
        renderPassBeginInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        commandCount++;
    }

    void CommandBuffer::endRenderPass() {
        vkCmdEndRenderPass(handle);
        commandCount++;
    }

    void CommandBuffer::nextSubPass() {
        vkCmdNextSubpass(handle, VK_SUBPASS_CONTENTS_INLINE);
        commandCount++;
    }

    void CommandBuffer::clearColorAttachments(const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor) {
        VkRect2D vkRect;
        vkRect.offset = { 0, 0 };
        vkRect.extent = { static_cast<uint32>(framebuffer->getDimensionsAndLayers().x), static_cast<uint32>(framebuffer->getDimensionsAndLayers().y) };

        VkClearRect clearRect;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        clearRect.rect = vkRect;

        VkClearAttachment vkClearAttchments[MAX_FRAMEBUFFER_ATTACHEMENTS];
        uint32 i = 0;
        for (; i < framebuffer->getRenderPass()->getColorAttachmentCount(); i++) {
            vkClearAttchments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkClearAttchments[i].colorAttachment = i;
            vkClearAttchments[i].clearValue.color = clearColor;
        }

        vkCmdClearAttachments(handle, i, vkClearAttchments, 1, &clearRect);
        commandCount++;
    }

    void CommandBuffer::clearDepthStencilAttachment(const Ref<Framebuffer> &framebuffer, const VkClearDepthStencilValue &clearValue) {
        VkRect2D vkRect;
        vkRect.offset = { 0, 0 };
        vkRect.extent = { static_cast<uint32>(framebuffer->getDimensionsAndLayers().x), static_cast<uint32>(framebuffer->getDimensionsAndLayers().y) };

        VkClearRect clearRect;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        clearRect.rect = vkRect;

        VkClearAttachment vkClearAttchments;
        vkClearAttchments.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        vkClearAttchments.colorAttachment = 0; //Irrelevant
        vkClearAttchments.clearValue.depthStencil = clearValue;

        vkCmdClearAttachments(handle, 1, &vkClearAttchments, 1, &clearRect);
        commandCount++;
    }

    void CommandBuffer::bindBuffer(const Ref<Buffer> &buffer, uint32 offset) {
        if (buffer->isVertex()) {
            VkBuffer vkBuffers [] = { buffer->getHandle().bufferHandle };
            VkDeviceSize offsets [] = { offset };
            vkCmdBindVertexBuffers(handle, 0, 1, vkBuffers, offsets);
        }
        else if (buffer->isIndex()) {
            VkBuffer vkBuffer = buffer->getHandle().bufferHandle;
            VkIndexType type = buffer->getLayout().begin()->getDataType() == DataType::Uint16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
            vkCmdBindIndexBuffer(handle, vkBuffer, offset, type);
        }
        commandCount++;
    }

    void CommandBuffer::bindPipelineState(const Ref<PipelineState> &pipelineState) {
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState->getHandle().pipeline);
        commandCount++;
    }

    void CommandBuffer::bindDescriptorSet(const Ref<DescriptorSet> &descriptorSet,
        const Ref<PipelineState> &pipelineState, uint32 setIndex,
        uint32 dynamicBufferOffsets [], uint32 dynamicBufferCount) {

        VkDescriptorSet descSets[] = { descriptorSet->getHandle() };
        vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineState->getHandle().pipelineLayout, setIndex,
            1, descSets, dynamicBufferCount, dynamicBufferOffsets);
        commandCount++;
    }

    void CommandBuffer::setPushConstants(const Ref<PipelineState> &pipelineState, VkShaderStageFlags shaderStageFlags, const void* data, uint32 size, uint32 offset) {
        vkCmdPushConstants(handle, pipelineState->getHandle().pipelineLayout, shaderStageFlags, offset, size, data);
        commandCount++;
    }

    void CommandBuffer::draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
        commandCount++;
    }

    void CommandBuffer::drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        commandCount++;
    }

    void CommandBuffer::setViewports(uint32 firstIndex, const VkViewport viewports[], uint32 viewportCount) {
        vkCmdSetViewport(handle, firstIndex, viewportCount, viewports);
        commandCount++;
    }

    void CommandBuffer::setScissorRects(uint32 firstIndex, const VkRect2D rects[], uint32 rectCount) {
        vkCmdSetScissor(handle, firstIndex, rectCount, rects);
        commandCount++;
    }

    void CommandBuffer::setDepthBias(float constantFactor, float clamp, float slopeFactor) {
        vkCmdSetDepthBias(handle, constantFactor, clamp, slopeFactor);
        commandCount++;
    }

    void CommandBuffer::pipelineBarrierTexture(const Ref<Texture> &texture) {
        GraphicsContext &context = getGraphicsContext();

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        if (texture->getFormat().isColor())
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        else if (texture->getFormat().isDepth() || texture->getFormat().isStencil())
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        //Making some assumptions here...
        if (texture->getFormat().isColor())
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        /*else if (texture.getFormat().isDepth())
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        else if (texture.getFormat().isStencil())
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;*/
        else if (texture->getFormat().isDepth() || texture->getFormat().isStencil())
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        uint32 graphicsQueueFamily = context.getDevice().getQueueContainer().graphics().getFamily().getIndex();
        imageMemoryBarrier.srcQueueFamilyIndex = graphicsQueueFamily;
        imageMemoryBarrier.dstQueueFamilyIndex = graphicsQueueFamily;

        imageMemoryBarrier.image = texture->getHandle().imageHandle;

        VkImageSubresourceRange subResourceRange;
        subResourceRange.aspectMask = 0;
        if (texture->getFormat().isColor())
            subResourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
        if (texture->getFormat().isDepth())
            subResourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (texture->getFormat().isStencil())
            subResourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        subResourceRange.baseMipLevel = 0;
        subResourceRange.levelCount = texture->getMipLevels();
        subResourceRange.baseArrayLayer = 0;
        subResourceRange.layerCount = texture->getLayers();

        imageMemoryBarrier.subresourceRange = subResourceRange;

        VkPipelineStageFlags srcStageMask;
        if (texture->getFormat().isColor())
            srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        else if (texture->getFormat().isDepth() || texture->getFormat().isStencil())
            srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        vkCmdPipelineBarrier(handle, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        commandCount++;
    }
}