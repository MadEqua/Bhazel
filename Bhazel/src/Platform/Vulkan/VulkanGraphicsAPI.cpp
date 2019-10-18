#include "bzpch.h"

#include "VulkanGraphicsApi.h"

#include "Platform/Vulkan/VulkanIncludes.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    VulkanGraphicsApi::VulkanGraphicsApi(VulkanContext &graphicsContext) :
        graphicsContext(graphicsContext) {
    }

    Ref<CommandBuffer> VulkanGraphicsApi::startRecording() {
        return startRecordingForFrame(graphicsContext.currentFrame, graphicsContext.getCurrentFrameFramebuffer());
    }

    Ref<CommandBuffer> VulkanGraphicsApi::startRecording(const Ref<Framebuffer> &framebuffer) {
        return startRecordingForFrame(graphicsContext.currentFrame, framebuffer);
    }

    Ref<CommandBuffer> VulkanGraphicsApi::startRecordingForFrame(uint32 frameIndex) {
        return startRecordingForFrame(frameIndex, graphicsContext.getFramebuffer(frameIndex));
    }

    Ref<CommandBuffer> VulkanGraphicsApi::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {

        //Begin a command buffer
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        auto &commandBufferRef = VulkanCommandBuffer::create(QueueProperty::Graphics, frameIndex);
        BZ_ASSERT_VK(vkBeginCommandBuffer(commandBufferRef->getNativeHandle(), &beginInfo));

        //Record a render pass
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(*framebuffer);

        //We know that the color attachments will be first and then the depthstencil
        VkClearValue clearValues[MAX_FRAMEBUFFER_ATTACHEMENTS];
        int i;
        for(i = 0; i < vulkanFramebuffer.getColorAttachmentCount(); ++i) {
            auto &attachmentClearValues = vulkanFramebuffer.getColorAttachment(i).description.clearValues;
            memcpy(clearValues[i].color.float32, &attachmentClearValues, sizeof(float) * 4);
        }
        if(vulkanFramebuffer.hasDepthStencilAttachment()) {
            auto &attachmentClearValues = vulkanFramebuffer.getDepthStencilAttachment()->description.clearValues;
            clearValues[i].depthStencil.depth = attachmentClearValues.floating.x;
            clearValues[i].depthStencil.stencil = attachmentClearValues.integer.y;
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanFramebuffer.getNativeHandle().renderPassHandle;
        renderPassBeginInfo.framebuffer = vulkanFramebuffer.getNativeHandle().frameBufferHandle;
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(vulkanFramebuffer.getDimensions().x), static_cast<uint32_t>(vulkanFramebuffer.getDimensions().y) };
        renderPassBeginInfo.clearValueCount = vulkanFramebuffer.getAttachmentCount();
        renderPassBeginInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(commandBufferRef->getNativeHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        return commandBufferRef;
    }

    void VulkanGraphicsApi::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        VkBuffer vkBuffers[] = { static_cast<const VulkanBuffer &>(*buffer).getNativeHandle() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vulkanCommandBuffer.getNativeHandle(), 0, 1, vkBuffers, offsets);
    }

    void VulkanGraphicsApi::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanBuffer = static_cast<const VulkanBuffer &>(*buffer);
        vkCmdBindIndexBuffer(vulkanCommandBuffer.getNativeHandle(), vulkanBuffer.getNativeHandle(), 0, VK_INDEX_TYPE_UINT16); //TODO index size
    }

    void VulkanGraphicsApi::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        vkCmdBindPipeline(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineState.getNativeHandle().pipeline);
    }

    void VulkanGraphicsApi::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        VkDescriptorSet descSets[] = { static_cast<const VulkanDescriptorSet &>(*descriptorSet).getNativeHandle() };
        vkCmdBindDescriptorSets(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, 
            vulkanPipelineState.getNativeHandle().pipelineLayout, 0, 1, descSets, 0, nullptr);
    }

    void VulkanGraphicsApi::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDraw(vulkanCommandBuffer.getNativeHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanGraphicsApi::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDrawIndexed(vulkanCommandBuffer.getNativeHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanGraphicsApi::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdEndRenderPass(vulkanCommandBuffer.getNativeHandle());
        BZ_ASSERT_VK(vkEndCommandBuffer(vulkanCommandBuffer.getNativeHandle()));
    }

    void VulkanGraphicsApi::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        const VulkanContext::FrameData *frameData = graphicsContext.frameData;
        uint32 currentFrame = graphicsContext.currentFrame;

        VkSemaphore waitSemaphores[] = { graphicsContext.frameData[graphicsContext.currentFrame].imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        VkCommandBuffer vkCommandBuffers[] = { vulkanCommandBuffer.getNativeHandle() };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = vkCommandBuffers;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        BZ_ASSERT_VK(vkWaitForFences(graphicsContext.device.getNativeHandle(), 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));

        BZ_ASSERT_VK(vkQueueSubmit(graphicsContext.device.getQueueContainer().graphics.getNativeHandle(), 1, &submitInfo, VK_NULL_HANDLE));
    }

    void VulkanGraphicsApi::endFrame() {
        const VulkanContext::FrameData *frameData = graphicsContext.frameData;
        uint32 currentFrame = graphicsContext.currentFrame;

        VkSemaphore signalSemaphores[] = { frameData[currentFrame].renderFinishedSemaphore };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 0;
        submitInfo.pCommandBuffers = nullptr;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        BZ_ASSERT_VK(vkWaitForFences(graphicsContext.device.getNativeHandle(), 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));
        BZ_ASSERT_VK(vkResetFences(graphicsContext.device.getNativeHandle(), 1, &frameData[currentFrame].inFlightFence));

        BZ_ASSERT_VK(vkQueueSubmit(graphicsContext.device.getQueueContainer().graphics.getNativeHandle(), 1, &submitInfo, frameData[currentFrame].inFlightFence));
    }
}