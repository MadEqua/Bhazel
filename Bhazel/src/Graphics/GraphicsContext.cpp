#include "bzpch.h"

#include "GraphicsContext.h"

#include "Core/Application.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/RenderPass.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <GLFW/glfw3.h>
#include <imgui.h>


namespace BZ {

    void GraphicsContext::init() {
        GLFWwindow *windowHandle = Application::get().getWindow().getNativeHandle();

        instance.init();
        surface.init(instance, *static_cast<GLFWwindow*>(windowHandle));

        const std::vector<const char *> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        physicalDevice.init(instance, surface, requiredDeviceExtensions);
        device.init(physicalDevice, requiredDeviceExtensions);

        //Init VulkanMemoryAllocator lib.
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice.getHandle();
        allocatorInfo.device = device.getHandle();
        vmaCreateAllocator(&allocatorInfo, &memoryAllocator);

        createFrameData();

        descriptorPool.init(device, { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64 }, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
                                      { VK_DESCRIPTOR_TYPE_SAMPLER, 64 }, { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64 } }, 128);

        //Create the RenderPass
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = VK_FORMAT_D24_UNORM_S8_UINT;
        depthStencilAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthStencilAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentDesc.clearValue.depthStencil.depth = 1.0f;
        depthStencilAttachmentDesc.clearValue.depthStencil.stencil = 0;

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };
        subPassDesc.depthStencilAttachmentsRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        mainRenderPass = RenderPass::create({ colorAttachmentDesc, depthStencilAttachmentDesc }, { subPassDesc });

        //TODO: get the dimensions from a better place
        glm::ivec2 dimensions;
        glfwGetWindowSize(static_cast<GLFWwindow*>(windowHandle), &dimensions.x, &dimensions.y);
        auto colorTexture = Texture2D::createRenderTarget(dimensions.x, dimensions.y, 1, colorAttachmentDesc.format);
        colorTextureView = TextureView::create(colorTexture);
        auto depthTexture = Texture2D::createRenderTarget(dimensions.x, dimensions.y, 1, depthStencilAttachmentDesc.format);
        depthTextureView = TextureView::create(depthTexture);

        mainFramebuffer = Framebuffer::create(mainRenderPass, { colorTextureView, depthTextureView }, glm::ivec3(dimensions.x, dimensions.y, 1));

        swapchain.init(device, surface);
    }

    void GraphicsContext::destroy() {
        colorTextureView.reset();
        depthTextureView.reset();
        mainFramebuffer.reset();
        mainRenderPass.reset();
        
        descriptorPool.destroy();

        swapchain.destroy();

        cleanupFrameData();

        vmaDestroyAllocator(memoryAllocator);
        device.destroy();

        surface.destroy();
        instance.destroy();
    }

    void GraphicsContext::beginFrame() {
        //Make sure that this frame has finished before reutilizing its data. If not, we are GPU bound and should block here.
        frameDatas[currentFrameIndex].renderFinishedFence.waitFor();
        frameDatas[currentFrameIndex].renderFinishedFence.reset();

        FrameData &frameData = frameDatas[currentFrameIndex];
        for(auto &familyAndPool : frameData.commandPoolsByFamily) {
            familyAndPool.second.reset();
        }

        swapchain.aquireImage(frameDatas[currentFrameIndex].imageAvailableSemaphore);
        pendingCommandBufferIndex = 0;

        stats = {};
    }

    void GraphicsContext::endFrame() {
        VkCommandBuffer vkCommandBuffers[MAX_COMMAND_BUFFERS_PER_FRAME];
        for(uint32 idx = 0; idx < pendingCommandBufferIndex; ++idx) {
            vkCommandBuffers[idx] = pendingCommandBuffers[idx].getHandle();
        }

        VkSemaphore waitSemaphores[] = { frameDatas[currentFrameIndex].imageAvailableSemaphore.getHandle() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { frameDatas[currentFrameIndex].renderFinishedSemaphore.getHandle() };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = pendingCommandBufferIndex;
        submitInfo.pCommandBuffers = vkCommandBuffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkFence fence = frameDatas[currentFrameIndex].renderFinishedFence.getHandle();

        //TODO: submiting always to graphics...
        BZ_ASSERT_VK(vkQueueSubmit(device.getQueueContainer().graphics().getHandle(), 1, &submitInfo, fence));

        swapchain.presentImage(frameDatas[currentFrameIndex].renderFinishedSemaphore);
        currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void GraphicsContext::submitCommandBuffers(const CommandBuffer commandBuffers[], uint32 count) {
        BZ_ASSERT_CORE(pendingCommandBufferIndex + count <= MAX_COMMAND_BUFFERS_PER_FRAME, "Exceeding maximum command buffers per frame!");

        for(uint32 i = 0; i < count; ++i) {
            pendingCommandBuffers[pendingCommandBufferIndex + i] = commandBuffers[i];
            stats.commandCount += commandBuffers[i].getCommandCount();
        }

        pendingCommandBufferIndex += count;
        stats.commandBufferCount += count;
    }

    void GraphicsContext::waitForDevice() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getHandle()));
    }

    void GraphicsContext::onWindowResize(const WindowResizedEvent& e) {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getHandle()));

        swapchain.recreate();
        cleanupFrameData();
        createFrameData();

        currentFrameIndex = 0;
    }

    CommandPool& GraphicsContext::getCurrentFrameCommandPool(QueueProperty property, bool exclusive) {
        const QueueContainer &queueContainer = exclusive ? device.getQueueContainerExclusive() : device.getQueueContainer();
        const Queue &queue = queueContainer.getQueueByProperty(property);
        return frameDatas[currentFrameIndex].commandPoolsByFamily[queue.getFamily().getIndex()];
    }

    void GraphicsContext::createFrameData() {
        std::set<uint32> familiesInUse = device.getQueueContainer().getFamilyIndexesInUse();
        std::set<uint32> exclusiveFamiliesInUse = device.getQueueContainerExclusive().getFamilyIndexesInUse();
        familiesInUse.insert(exclusiveFamiliesInUse.begin(), exclusiveFamiliesInUse.end());

        for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            frameDatas[i].imageAvailableSemaphore.init(device);
            frameDatas[i].renderFinishedSemaphore.init(device);
            frameDatas[i].renderFinishedFence.init(device, true);

            for(uint32 famIdx : familiesInUse) {
                frameDatas[i].commandPoolsByFamily[famIdx].init(device, famIdx);
            }
        }
    }

    void GraphicsContext::cleanupFrameData() {
        for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            frameDatas[i].imageAvailableSemaphore.destroy();
            frameDatas[i].renderFinishedSemaphore.destroy();
            frameDatas[i].renderFinishedFence.destroy();

            for (auto &pair : frameDatas[i].commandPoolsByFamily) {
                pair.second.destroy();
            }
        }
    }

    void GraphicsContext::onImGuiRender(const FrameStats &frameStats) {
        stats.frameStats = frameStats;

        statsTimeAcumMs += frameStats.lastFrameTime.asMillisecondsUint32();
        if(statsTimeAcumMs >= statsRefreshPeriodMs) {
            statsTimeAcumMs = 0;
            visibleStats = stats;
            frameTimeHistory[frameTimeHistoryIdx] = frameStats.lastFrameTime.asMillisecondsFloat();
            frameTimeHistoryIdx = (frameTimeHistoryIdx + 1) % FRAME_HISTORY_SIZE;
        }

        if(ImGui::Begin("Graphics")) {
            ImGui::Text("FrameStats:");
            ImGui::Text("Last Frame Time: %.3f ms.", visibleStats.frameStats.lastFrameTime.asMillisecondsFloat());
            ImGui::Text("FPS: %.3f.", 1.0f / visibleStats.frameStats.lastFrameTime.asSeconds());
            //ImGui::Separator();
            ImGui::Text("Avg Frame Time: %.3f ms.", visibleStats.frameStats.runningTime.asMillisecondsFloat() / static_cast<float>(visibleStats.frameStats.frameCount));
            ImGui::Text("Avg FPS: %.3f.", static_cast<float>(visibleStats.frameStats.frameCount) / visibleStats.frameStats.runningTime.asSeconds());
            //ImGui::Separator();
            ImGui::Text("Frame Count: %d.", visibleStats.frameStats.frameCount);
            ImGui::Text("Running Time: %.3f seconds.", visibleStats.frameStats.runningTime.asSeconds());
            ImGui::Separator();

            ImGui::Text("Stats:");
            ImGui::Text("CommandBuffer Count: %d", visibleStats.commandBufferCount);
            ImGui::Text("Command Count: %d", visibleStats.commandCount);
            ImGui::Separator();

            ImGui::PlotLines("Frame Times", frameTimeHistory, FRAME_HISTORY_SIZE, frameTimeHistoryIdx, "ms", 0.0f, 50.0f, ImVec2(0, 80));
            ImGui::Separator();

            ImGui::SliderInt("Refresh period ms", reinterpret_cast<int*>(&statsRefreshPeriodMs), 0, 1000);
        }
        ImGui::End();
    }
}
