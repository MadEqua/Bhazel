#include "bzpch.h"

#include "VulkanRendererApi.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"

#include "Bhazel/Platform/Vulkan/VulkanCommandBuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanBuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanFramebuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanPipelineState.h"
#include "Bhazel/Platform/Vulkan/VulkanDescriptorSet.h"


namespace BZ {

    VulkanRendererApi::VulkanRendererApi(VulkanContext &graphicsContext) :
        graphicsContext(graphicsContext) {
    }

    Ref<CommandBuffer> VulkanRendererApi::startRecording() {
        return startRecordingForFrame(graphicsContext.currentFrame, graphicsContext.swapchainFramebuffers[graphicsContext.currentFrame]);
    }

    Ref<CommandBuffer> VulkanRendererApi::startRecording(const Ref<Framebuffer> &framebuffer) {
        return startRecordingForFrame(graphicsContext.currentFrame, framebuffer);
    }

    Ref<CommandBuffer> VulkanRendererApi::startRecordingForFrame(uint32 frameIndex) {
        return startRecordingForFrame(frameIndex, graphicsContext.swapchainFramebuffers[frameIndex]);
    }

    Ref<CommandBuffer> VulkanRendererApi::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {

        //Begin a command buffer
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        auto &commandBufferRef = VulkanCommandBuffer::create(RenderQueueFamily::Graphics, frameIndex);
        BZ_ASSERT_VK(vkBeginCommandBuffer(commandBufferRef->getNativeHandle(), &beginInfo));

        //Record a render pass
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(*framebuffer);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanFramebuffer.getNativeHandle().renderPassHandle;
        renderPassBeginInfo.framebuffer = vulkanFramebuffer.getNativeHandle().frameBufferHandle;
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = { 1280, 800 }; //TODO
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; //TODO
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBufferRef->getNativeHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        return commandBufferRef;
    }

    void VulkanRendererApi::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        VkBuffer vkBuffers[] = { static_cast<const VulkanBuffer &>(*buffer).getNativeHandle() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vulkanCommandBuffer.getNativeHandle(), 0, 1, vkBuffers, offsets);
    }

    void VulkanRendererApi::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanBuffer = static_cast<const VulkanBuffer &>(*buffer);
        vkCmdBindIndexBuffer(vulkanCommandBuffer.getNativeHandle(), vulkanBuffer.getNativeHandle(), 0, VK_INDEX_TYPE_UINT16); //TODO index size
    }

    void VulkanRendererApi::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        vkCmdBindPipeline(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineState.getNativeHandle().pipeline);
    }

    void VulkanRendererApi::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        VkDescriptorSet descSets[] = { static_cast<const VulkanDescriptorSet &>(*descriptorSet).getNativeHandle() };
        vkCmdBindDescriptorSets(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, 
            vulkanPipelineState.getNativeHandle().pipelineLayout, 0, 1, descSets, 0, nullptr);
    }

    void VulkanRendererApi::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDraw(vulkanCommandBuffer.getNativeHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanRendererApi::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDrawIndexed(vulkanCommandBuffer.getNativeHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanRendererApi::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdEndRenderPass(vulkanCommandBuffer.getNativeHandle());
        BZ_ASSERT_VK(vkEndCommandBuffer(vulkanCommandBuffer.getNativeHandle()));
    }

    void VulkanRendererApi::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
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

        BZ_ASSERT_VK(vkWaitForFences(graphicsContext.device, 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));
        BZ_ASSERT_VK(vkResetFences(graphicsContext.device, 1, &frameData[currentFrame].inFlightFence));

        BZ_ASSERT_VK(vkQueueSubmit(graphicsContext.graphicsQueue, 1, &submitInfo, frameData[currentFrame].inFlightFence));
    }

    void VulkanRendererApi::endFrame() {
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

        BZ_ASSERT_VK(vkWaitForFences(graphicsContext.device, 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));
        BZ_ASSERT_VK(vkResetFences(graphicsContext.device, 1, &frameData[currentFrame].inFlightFence));

        BZ_ASSERT_VK(vkQueueSubmit(graphicsContext.graphicsQueue, 1, &submitInfo, frameData[currentFrame].inFlightFence));
    }
}