#include "bzpch.h"

#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    Ref<VulkanCommandBuffer> VulkanCommandBuffer::wrap(VkCommandBuffer vkCommandBuffer) {
        return MakeRef<VulkanCommandBuffer>(vkCommandBuffer);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer vkCommandBuffer) {
        nativeHandle = vkCommandBuffer;
    }

    void VulkanCommandBuffer::begin(const Ref<Framebuffer> &framebuffer) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //Disallowing command buffer reusage
        beginInfo.pInheritanceInfo = nullptr;

        BZ_ASSERT_VK(vkBeginCommandBuffer(nativeHandle, &beginInfo));

        //Record a render pass. TODO: this should be separated
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(*framebuffer);

        //We know that the color attachments will be first and then the depthstencil
        VkClearValue clearValues[MAX_FRAMEBUFFER_ATTACHEMENTS];
        uint32 i;
        for(i = 0; i < vulkanFramebuffer.getColorAttachmentCount(); ++i) {
            const auto &att = vulkanFramebuffer.getColorAttachment(i);
            if(att.description.loadOperatorColorAndDepth == LoadOperation::Clear) {
                auto &attachmentClearValues = att.description.clearValues;
                memcpy(clearValues[i].color.float32, &attachmentClearValues, sizeof(float) * 4);
            }
        }
        if(vulkanFramebuffer.hasDepthStencilAttachment()) {
            const auto &att = vulkanFramebuffer.getDepthStencilAttachment();
            if(att->description.loadOperatorStencil == LoadOperation::Clear) {
                auto &attachmentClearValues = att->description.clearValues;
                clearValues[i].depthStencil.depth = attachmentClearValues.floating.x;
                clearValues[i].depthStencil.stencil = attachmentClearValues.integer.y;
            }
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanFramebuffer.getNativeHandle().renderPassHandle;
        renderPassBeginInfo.framebuffer = vulkanFramebuffer.getNativeHandle().frameBufferHandle;
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(vulkanFramebuffer.getDimensions().x), static_cast<uint32_t>(vulkanFramebuffer.getDimensions().y) };
        renderPassBeginInfo.clearValueCount = vulkanFramebuffer.getAttachmentCount();
        renderPassBeginInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(nativeHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::end() {
        vkCmdEndRenderPass(nativeHandle);
        BZ_ASSERT_VK(vkEndCommandBuffer(nativeHandle));
    }

    void VulkanCommandBuffer::clearColorAttachments(const Framebuffer &framebuffer, const ClearValues &clearColor) {
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(framebuffer);

        VkRect2D vkRect;
        vkRect.offset = { 0, 0 };
        vkRect.extent = { static_cast<uint32>(framebuffer.getDimensions().x), static_cast<uint32>(framebuffer.getDimensions().y) };

        VkClearRect clearRect;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        clearRect.rect = vkRect;

        VkClearAttachment vkClearAttchments[MAX_FRAMEBUFFER_ATTACHEMENTS];
        for(uint32 i = 0; i < framebuffer.getColorAttachmentCount(); i++) {
            vkClearAttchments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkClearAttchments[i].colorAttachment = i;
            memcpy(vkClearAttchments[i].clearValue.color.float32, &clearColor, sizeof(float) * 4);
        }

        vkCmdClearAttachments(nativeHandle, vulkanFramebuffer.getColorAttachmentCount(), vkClearAttchments, 1, &clearRect);
    }

    void VulkanCommandBuffer::clearDepthStencilAttachment(const Framebuffer &framebuffer, const ClearValues &clearValue) {
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(framebuffer);

        VkRect2D vkRect;
        vkRect.offset = { 0, 0 };
        vkRect.extent = { static_cast<uint32>(framebuffer.getDimensions().x), static_cast<uint32>(framebuffer.getDimensions().y) };

        VkClearRect clearRect;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        clearRect.rect = vkRect;

        VkClearAttachment vkClearAttchments;
        vkClearAttchments.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        vkClearAttchments.colorAttachment = 0; //Irrelevant
        vkClearAttchments.clearValue.depthStencil.depth = clearValue.floating.x;
        vkClearAttchments.clearValue.depthStencil.stencil = clearValue.integer.y;

        vkCmdClearAttachments(nativeHandle, 1, &vkClearAttchments, 1, &clearRect);
    }

    void VulkanCommandBuffer::bindBuffer(const Buffer &buffer, uint32 offset) {
        if(buffer.getType() == BufferType::Vertex) {
            VkBuffer vkBuffers[] = { static_cast<const VulkanBuffer &>(buffer).getNativeHandle() };
            VkDeviceSize offsets[] = { offset };
            vkCmdBindVertexBuffers(nativeHandle, 0, 1, vkBuffers, offsets);
        }
        else if(buffer.getType() == BufferType::Index) {
            VkBuffer vkBuffer = static_cast<const VulkanBuffer &>(buffer).getNativeHandle();
            vkCmdBindIndexBuffer(nativeHandle, vkBuffer, offset, VK_INDEX_TYPE_UINT16); //TODO index size
        }
    }

    void VulkanCommandBuffer::bindPipelineState(const PipelineState &pipelineState) {
        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(pipelineState);
        vkCmdBindPipeline(nativeHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineState.getNativeHandle().pipeline);
    }

    void VulkanCommandBuffer::bindDescriptorSet(const DescriptorSet &descriptorSet,
                                                const PipelineState &pipelineState, uint32 setIndex,
                                                uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) {
        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(pipelineState);
        VkDescriptorSet descSets[] = { static_cast<const VulkanDescriptorSet &>(descriptorSet).getNativeHandle() };
        vkCmdBindDescriptorSets(nativeHandle, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vulkanPipelineState.getNativeHandle().pipelineLayout, setIndex,
                                1, descSets, dynamicBufferCount, dynamicBufferOffsets);
    }

    void VulkanCommandBuffer::draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        vkCmdDraw(nativeHandle, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandBuffer::drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        vkCmdDrawIndexed(nativeHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanCommandBuffer::setViewports(uint32 firstIndex, const Command::Viewport viewports[], uint32 viewportCount) {
        VkViewport vkViewports[MAX_VIEWPORTS];
        for(uint32 i = 0; i < viewportCount; ++i) {
            vkViewports[i].x = viewports[i].rect.left;
            vkViewports[i].y = viewports[i].rect.top;
            vkViewports[i].width = viewports[i].rect.width;
            vkViewports[i].height = viewports[i].rect.height;
            vkViewports[i].minDepth = viewports[i].minDepth;
            vkViewports[i].maxDepth = viewports[i].maxDepth;
        }
        vkCmdSetViewport(nativeHandle, firstIndex, viewportCount, vkViewports);
    }

    void VulkanCommandBuffer::setScissorRects(uint32 firstIndex, const Command::ScissorRect rects[], uint32 rectCount) {
        VkRect2D vkRects[MAX_VIEWPORTS];
        for(uint32 i = 0; i < rectCount; ++i) {
            vkRects[i].offset.x = rects[i].rect.left;
            vkRects[i].offset.y = rects[i].rect.top;
            vkRects[i].extent.width = rects[i].rect.width;
            vkRects[i].extent.height = rects[i].rect.height;
        }
        vkCmdSetScissor(nativeHandle, firstIndex, rectCount, vkRects);
    }

}