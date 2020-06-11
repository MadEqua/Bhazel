#include "bzpch.h"

#include "CommandBuffer.h"

#include "Core/Engine.h"

#include "Graphics/Buffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/PipelineState.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"

#include "Graphics/Internal/QueryPool.h"


namespace BZ {

CommandBuffer &CommandBuffer::getAndBegin(QueueProperty queueProperty) {
    auto &buf = BZ_GRAPHICS_CTX.getCurrentFrameCommandPool(queueProperty).getCommandBuffer();
    buf.begin();
    return buf;
}

void CommandBuffer::init(VkCommandBuffer vkCommandBuffer, uint32 queueFamilyIndex) {
    this->queueFamilyIndex = queueFamilyIndex;

    handle = vkCommandBuffer;
    commandCount = 0;
}

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Disallowing command buffer reusage
    beginInfo.pInheritanceInfo = nullptr;

    BZ_ASSERT_VK(vkBeginCommandBuffer(handle, &beginInfo));

    commandCount = 0;
}

void CommandBuffer::end() {
    BZ_ASSERT_VK(vkEndCommandBuffer(handle));
}

void CommandBuffer::endAndSubmit(bool waitForImageAvailable, bool signalFrameEnd) {
    end();
    submit(waitForImageAvailable, signalFrameEnd);
}

void CommandBuffer::submit(bool waitForImageAvailable, bool signalFrameEnd) {
    const CommandBuffer *arr[] = { this };
    BZ_GRAPHICS_CTX.submitCommandBuffers(arr, 1, waitForImageAvailable, signalFrameEnd);
}

void CommandBuffer::beginRenderPass(const Ref<RenderPass> &renderPass, const Ref<Framebuffer> &framebuffer) {
    // auto &renderPass = framebuffer->getRenderPass();

    std::vector<VkClearValue> clearValues(renderPass->getAttachmentCount());
    for (uint32 i = 0; i < renderPass->getAttachmentCount(); ++i) {
        const auto &attDesc = renderPass->getAttachmentDescription(i);
        if (attDesc.loadOperatorColorAndDepth == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValues[i].color = attDesc.clearValue.color;
            clearValues[i].depthStencil.depth = attDesc.clearValue.depthStencil.depth;
        }
        if (attDesc.loadOperatorStencil == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValues[i].depthStencil.stencil = attDesc.clearValue.depthStencil.stencil;
        }
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass->getHandle();
    renderPassBeginInfo.framebuffer = framebuffer->getHandle();
    renderPassBeginInfo.renderArea.offset = {};
    renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(framebuffer->getDimensionsAndLayers().x),
                                              static_cast<uint32_t>(framebuffer->getDimensionsAndLayers().y) };
    renderPassBeginInfo.clearValueCount = renderPass->getAttachmentCount();
    renderPassBeginInfo.pClearValues = clearValues.data();
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
    vkRect.extent = { static_cast<uint32>(framebuffer->getDimensionsAndLayers().x),
                      static_cast<uint32>(framebuffer->getDimensionsAndLayers().y) };

    VkClearRect clearRect;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = static_cast<uint32>(framebuffer->getDimensionsAndLayers().z);
    clearRect.rect = vkRect;

    std::vector<VkClearAttachment> vkClearAttchments(framebuffer->getColorAttachmentCount());
    uint32 i = 0;
    for (; i < framebuffer->getColorAttachmentCount(); i++) {
        vkClearAttchments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkClearAttchments[i].colorAttachment = i;
        vkClearAttchments[i].clearValue.color = clearColor;
    }

    vkCmdClearAttachments(handle, i, vkClearAttchments.data(), 1, &clearRect);
    commandCount++;
}

void CommandBuffer::clearDepthStencilAttachment(const Ref<Framebuffer> &framebuffer,
                                                const VkClearDepthStencilValue &clearValue) {
    VkRect2D vkRect;
    vkRect.offset = { 0, 0 };
    vkRect.extent = { static_cast<uint32>(framebuffer->getDimensionsAndLayers().x),
                      static_cast<uint32>(framebuffer->getDimensionsAndLayers().y) };

    VkClearRect clearRect;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = static_cast<uint32>(framebuffer->getDimensionsAndLayers().z);
    clearRect.rect = vkRect;

    VkClearAttachment vkClearAttchments;
    vkClearAttchments.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkClearAttchments.colorAttachment = 0; // Irrelevant
    vkClearAttchments.clearValue.depthStencil = clearValue;

    vkCmdClearAttachments(handle, 1, &vkClearAttchments, 1, &clearRect);
    commandCount++;
}

void CommandBuffer::bindBuffer(const Ref<Buffer> &buffer, uint32 offset) {
    BZ_ASSERT_CORE(buffer->isVertex() || buffer->isIndex(), "Invalid Buffer type!");

    uint32 realOffset = buffer->getCurrentBaseOfReplicaOffset() + offset;

    if (buffer->isVertex()) {
        VkBuffer vkBuffers[] = { buffer->getHandle().bufferHandle };
        VkDeviceSize offsets[] = { realOffset };
        vkCmdBindVertexBuffers(handle, 0, 1, vkBuffers, offsets);
    }
    else if (buffer->isIndex()) {
        VkBuffer vkBuffer = buffer->getHandle().bufferHandle;
        VkIndexType type = buffer->getLayout().begin()->getDataType() == DataType::Uint16 ? VK_INDEX_TYPE_UINT16
                                                                                          : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(handle, vkBuffer, realOffset, type);
    }
    commandCount++;
}

void CommandBuffer::bindPipelineState(const Ref<PipelineState> &pipelineState) {
    vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState->getHandle());
    commandCount++;
}

void CommandBuffer::bindDescriptorSet(const DescriptorSet &descriptorSet, const Ref<PipelineLayout> &pipelineLayout,
                                      uint32 setIndex, uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) {

    constexpr uint32 MAX_DESCRIPTOR_DYNAMIC_OFFSETS = 8;

    BZ_ASSERT_CORE(dynamicBufferCount <= MAX_DESCRIPTOR_DYNAMIC_OFFSETS &&
                       dynamicBufferCount <= descriptorSet.getDynamicBufferCount(),
                   "Invalid dynamicBufferCount: {}!", dynamicBufferCount);

    // Mix correctly the dynamicBufferOffsets coming from the user with the ones that the engine
    // needs to send behind the scenes for dynamic buffers.
    uint32 finalDynamicBufferOffsets[MAX_DESCRIPTOR_DYNAMIC_OFFSETS];
    uint32 index = 0;
    uint32 userIndex = 0;
    uint32 binding = 0;
    for (const auto &desc : descriptorSet.getLayout()->getDescriptorDescs()) {
        if (desc.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            desc.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            const auto *dynBufferData = descriptorSet.getDynamicBufferDataByBinding(binding);
            BZ_ASSERT_CORE(dynBufferData, "Non-existent binding should not happen!");

            for (uint32 i = 0; i < static_cast<uint32>(dynBufferData->buffers.size()); ++i) {
                finalDynamicBufferOffsets[index] = dynBufferData->buffers[i]->getCurrentBaseOfReplicaOffset();
                if (userIndex < dynamicBufferCount) {
                    finalDynamicBufferOffsets[index] += dynamicBufferOffsets[userIndex++];
                }
                else if (dynamicBufferCount > 0) {
                    BZ_LOG_CORE_WARN("Graphics::bindDescriptorSet(): there are more dynamic "
                                     "buffers on the DescriptorSet than the number of offsets sent "
                                     "to the bind function. Might be an error.");
                }
                index++;
            }
        }
        binding++;
    }

    VkDescriptorSet descSets[] = { descriptorSet.getHandle() };
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->getHandle(), setIndex, 1, descSets,
                            index, finalDynamicBufferOffsets);
    commandCount++;
}

void CommandBuffer::setPushConstants(const Ref<PipelineLayout> &pipelineLayout, VkShaderStageFlags shaderStageFlags,
                                     const void *data, uint32 offset, uint32 size) {
    constexpr uint32 MAX_PUSH_CONSTANT_SIZE = 128;

    BZ_ASSERT_CORE(size % 4 == 0, "Size must be a multiple of 4!");
    BZ_ASSERT_CORE(offset % 4 == 0, "Offset must be a multiple of 4!");
    BZ_ASSERT_CORE(size <= MAX_PUSH_CONSTANT_SIZE,
                   "Push constant size must be less or equal than {}. Sending size: {}!", MAX_PUSH_CONSTANT_SIZE, size);

    vkCmdPushConstants(handle, pipelineLayout->getHandle(), shaderStageFlags, offset, size, data);
    commandCount++;
}

void CommandBuffer::draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
    vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
    commandCount++;
}

void CommandBuffer::drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset,
                                uint32 firstInstance) {
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

void CommandBuffer::pipelineBarrierMemory(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                                          VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
    VkMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(handle, srcStage, dstStage,
                         0, // TODO: this may be a parameter
                         1, &barrier, 0, nullptr, 0, nullptr);

    commandCount++;
}

void CommandBuffer::pipelineBarrierTexture(const Ref<Texture> &texture, VkPipelineStageFlags srcStage,
                                           VkPipelineStageFlags dstStage, VkAccessFlags srcAccessMask,
                                           VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                           VkImageLayout newLayout, uint32 baseMipLevel, uint32 mipLevels) {

    pipelineBarrierTexture(*texture, srcStage, dstStage, srcAccessMask, dstAccessMask, oldLayout, newLayout,
                           baseMipLevel, mipLevels);
}

void CommandBuffer::pipelineBarrierTexture(const Texture &texture, VkPipelineStageFlags srcStage,
                                           VkPipelineStageFlags dstStage, VkAccessFlags srcAccessMask,
                                           VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                           VkImageLayout newLayout, uint32 baseMipLevel, uint32 mipLevels) {

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle().imageHandle;

    barrier.subresourceRange.aspectMask = 0;
    if (texture.getFormat().isColor())
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (texture.getFormat().isDepth())
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (texture.getFormat().isStencil())
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = mipLevels;
    // TODO: those may be parameters
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = texture.getLayers();

    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(handle, srcStage, dstStage,
                         0, // TODO: this may be a parameter
                         0, nullptr, 0, nullptr, 1, &barrier);

    commandCount++;
}

void CommandBuffer::copyBufferToBuffer(const Ref<Buffer> &src, const Ref<Buffer> &dst, VkBufferCopy regions[],
                                       uint32 regionCount) {
    copyBufferToBuffer(*src, *dst, regions, regionCount);
}

void CommandBuffer::copyBufferToBuffer(const Buffer &src, const Buffer &dst, VkBufferCopy regions[],
                                       uint32 regionCount) {
    vkCmdCopyBuffer(handle, src.getHandle().bufferHandle, dst.getHandle().bufferHandle, regionCount, regions);
    commandCount++;
}

void CommandBuffer::copyBufferToTexture(const Ref<Buffer> &src, const Ref<Texture> &dst, VkImageLayout imageLayout,
                                        const VkBufferImageCopy regions[], uint32 regionCount) {
    copyBufferToTexture(*src, *dst, imageLayout, regions, regionCount);
}

void CommandBuffer::copyBufferToTexture(const Buffer &src, const Texture &dst, VkImageLayout imageLayout,
                                        const VkBufferImageCopy regions[], uint32 regionCount) {
    vkCmdCopyBufferToImage(handle, src.getHandle().bufferHandle, dst.getHandle().imageHandle, imageLayout, regionCount,
                           regions);
    commandCount++;
}

void CommandBuffer::blitTexture(const Ref<Texture> &src, const Ref<Texture> &dst, VkImageLayout srcLayout,
                                VkImageLayout dstLayout, VkImageBlit blit[], uint32 blitCount, VkFilter filter) {
    blitTexture(*src, *dst, srcLayout, dstLayout, blit, blitCount, filter);
}

void CommandBuffer::blitTexture(const Texture &src, const Texture &dst, VkImageLayout srcLayout,
                                VkImageLayout dstLayout, VkImageBlit blit[], uint32 blitCount, VkFilter filter) {
    vkCmdBlitImage(handle, src.getHandle().imageHandle, srcLayout, dst.getHandle().imageHandle, dstLayout, blitCount,
                   blit, filter);
    commandCount++;
}

void CommandBuffer::resetQueryPool(const QueryPool &pool, uint32 firstQuery, uint32 queryCount) {
    vkCmdResetQueryPool(handle, pool.getHandle(), firstQuery, queryCount);
    commandCount++;
}

void CommandBuffer::beginQuery(const QueryPool &pool, uint32 queryIndex, VkQueryControlFlags controlFlags) {
    vkCmdBeginQuery(handle, pool.getHandle(), queryIndex, controlFlags);
    commandCount++;
}

void CommandBuffer::endQuery(const QueryPool &pool, uint32 queryIndex) {
    vkCmdEndQuery(handle, pool.getHandle(), queryIndex);
    commandCount++;
}

void CommandBuffer::writeTimestamp(VkPipelineStageFlagBits pipelineStage, const QueryPool &pool, uint32 queryIndex) {
    vkCmdWriteTimestamp(handle, pipelineStage, pool.getHandle(), queryIndex);
    commandCount++;
}

void CommandBuffer::copyQueryPoolResults(const QueryPool &pool, uint32 firstQuery, uint32 queryCount,
                                         const Ref<Buffer> &dstBuffer, uint32 dstOffset, uint32 stride,
                                         VkQueryResultFlags flags) {
    vkCmdCopyQueryPoolResults(handle, pool.getHandle(), firstQuery, queryCount, dstBuffer->getHandle().bufferHandle,
                              dstOffset, stride, flags);
    commandCount++;
}

#ifdef BZ_GRAPHICS_DEBUG
void CommandBuffer::beginDebugLabel(const char *label, const glm::vec4 &color) {
    VkDebugUtilsLabelEXT labelStruct = {};
    labelStruct.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelStruct.pLabelName = label;
    memcpy(&labelStruct.color, &color[0], sizeof(float[4]));

    static auto func =
        BZ_GRAPHICS_CTX.getExtensionFunction<PFN_vkCmdBeginDebugUtilsLabelEXT>("vkCmdBeginDebugUtilsLabelEXT");
    func(handle, &labelStruct);
}

void CommandBuffer::endDebugLabel() {
    static auto func =
        BZ_GRAPHICS_CTX.getExtensionFunction<PFN_vkCmdEndDebugUtilsLabelEXT>("vkCmdEndDebugUtilsLabelEXT");
    func(handle);
}

void CommandBuffer::insertDebugLabel(const char *label, const glm::vec4 &color) {
    VkDebugUtilsLabelEXT labelStruct = {};
    labelStruct.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelStruct.pLabelName = label;
    memcpy(&labelStruct.color, &color[0], sizeof(float[4]));

    static auto func =
        BZ_GRAPHICS_CTX.getExtensionFunction<PFN_vkCmdInsertDebugUtilsLabelEXT>("vkCmdInsertDebugUtilsLabelEXT");
    func(handle, &labelStruct);
}
#endif
}