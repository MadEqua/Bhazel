#include "bzpch.h"

#include "GraphicsContext.h"

#include "Core/Application.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/DescriptorSet.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <GLFW/glfw3.h>
#include <imgui.h>


namespace BZ {

void GraphicsContext::init() {
    GLFWwindow *windowHandle = Application::get().getWindow().getNativeHandle();

    instance.init();
    surface.init(instance, *static_cast<GLFWwindow *>(windowHandle));

    const std::vector<const char *> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    physicalDevice.init(instance, surface, requiredDeviceExtensions);
    device.init(physicalDevice, requiredDeviceExtensions);

    // Init VulkanMemoryAllocator lib.
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice.getHandle();
    allocatorInfo.device = device.getHandle();
    vmaCreateAllocator(&allocatorInfo, &memoryAllocator);

    createFrameData();

    descriptorPool.init(device,
                        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64 },
                          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
                          { VK_DESCRIPTOR_TYPE_SAMPLER, 64 },
                          { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64 } },
                        128);

    swapchain.init(device, surface);
    stats = {};
}

void GraphicsContext::destroy() {
    descriptorPool.destroy();

    swapchain.destroy();

    cleanupFrameData();

    vmaDestroyAllocator(memoryAllocator);
    device.destroy();

    surface.destroy();
    instance.destroy();
}

void GraphicsContext::beginFrame() {
    stats.cpuBound = frameDatas[currentFrameIndex].renderFinishedFence->isSignaled();

    // Make sure that this frame has finished before reutilizing its data. If not, we are GPU bound and should block
    // here.
    frameDatas[currentFrameIndex].renderFinishedFence->waitFor();
    frameDatas[currentFrameIndex].renderFinishedFence->reset();

    FrameData &frameData = frameDatas[currentFrameIndex];
    for (auto &familyAndPool : frameData.commandPoolsByFamily) {
        familyAndPool.second.reset();
    }

    swapchain.aquireImage(frameDatas[currentFrameIndex].imageAvailableSemaphore);

#ifdef BZ_GRAPHICS_DEBUG
    // Read data from last timestamp.
    const QueueFamily &graphicsFam = device.getQueueContainer().graphics().getFamily();
    if (graphicsFam.canUseTimestamps()) {
        uint64 timestampTicks[TIMESTAMP_QUERY_COUNT] = {};
        frameDatas[currentFrameIndex].queryPool.getResults(0, 2, sizeof(timestampTicks), &timestampTicks,
                                                           sizeof(uint64), VK_QUERY_RESULT_64_BIT);
        uint32 validBits = graphicsFam.getTimestampValidBits();
        for (int i = 0; i < TIMESTAMP_QUERY_COUNT; ++i) {
            timestampTicks[i] &= BZ_BIT_MASK(uint64, validBits);
        }

        float timestampPeriod = device.getPhysicalDevice().getLimits().timestampPeriod;
        stats.frameTimeGpu =
            static_cast<uint64>(static_cast<float>(timestampTicks[1] - timestampTicks[0]) * timestampPeriod);

        // Inject timestamp at frame start.
        CommandBuffer &cmdBuf = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        cmdBuf.resetQueryPool(frameDatas[currentFrameIndex].queryPool, 0, TIMESTAMP_QUERY_COUNT);
        cmdBuf.writeTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, frameDatas[currentFrameIndex].queryPool, 0);
        cmdBuf.endAndSubmit();
    }
#endif
}

void GraphicsContext::endFrame() {
    swapchain.presentImage(frameDatas[currentFrameIndex].renderFinishedSemaphore);
    currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    stats.frameCount++;
}

void GraphicsContext::submitCommandBuffers(const CommandBuffer *commandBuffers[], uint32 commandBuffersCount,
                                           bool waitForImageAvailable, bool signalFrameEnd) {

    Ref<Semaphore> waitSemaphores[] = { frameDatas[currentFrameIndex].imageAvailableSemaphore };
    VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    Ref<Semaphore> signalSemaphores[] = { frameDatas[currentFrameIndex].renderFinishedSemaphore };
    Ref<Fence> signalFence;
    if (signalFrameEnd)
        signalFence = frameDatas[currentFrameIndex].renderFinishedFence;

    submitCommandBuffers(commandBuffers, commandBuffersCount, waitSemaphores, waitFlags, waitForImageAvailable ? 1 : 0,
                         signalSemaphores, signalFrameEnd ? 1 : 0, signalFence);
}

void GraphicsContext::submitCommandBuffers(const CommandBuffer *commandBuffers[], uint32 commandBuffersCount,
                                           const Ref<Semaphore> semaphoresToWaitFor[],
                                           VkPipelineStageFlags waitStages[], uint32 semaphoresToWaitForCount,
                                           const Ref<Semaphore> semaphoresToSignal[], uint32 semaphoresToSignalCount,
                                           const Ref<Fence> &fenceToSignal) {

    BZ_ASSERT_CORE(commandBuffersCount > 0, "Invalid commandBuffersCount!");
    BZ_ASSERT_CORE(commandBuffersCount <= MAX_COMMAND_BUFFERS_PER_SUBMIT,
                   "commandBuffersCount needs to be <= than MAX_COMMAND_BUFFERS_PER_SUBMIT!");
    BZ_ASSERT_CORE(semaphoresToWaitForCount < MAX_SEMAPHORES_PER_SUBMIT,
                   "semaphoresToWaitForCount needs to be <= than MAX_SEMAPHORES_PER_SUBMIT!");
    BZ_ASSERT_CORE(semaphoresToSignalCount < MAX_SEMAPHORES_PER_SUBMIT,
                   "semaphoresToSignalCount needs to be <= than MAX_SEMAPHORES_PER_SUBMIT!");

    VkCommandBuffer vkCommandBuffers[MAX_COMMAND_BUFFERS_PER_SUBMIT + 1];
    for (uint32 idx = 0; idx < commandBuffersCount; ++idx) {
        vkCommandBuffers[idx] = commandBuffers[idx]->getHandle();
        stats.commandCount += commandBuffers[idx]->getCommandCount();
    }

#ifdef BZ_GRAPHICS_DEBUG
    // Inject timestamp CommandBuffer after last frame CommandBuffer.
    const QueueFamily &graphicsFam = device.getQueueContainer().graphics().getFamily();
    if (graphicsFam.canUseTimestamps() && fenceToSignal) {
        CommandBuffer &cmdBuf = CommandBuffer::getAndBegin(QueueProperty::Graphics);
        cmdBuf.writeTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, frameDatas[currentFrameIndex].queryPool, 1);
        cmdBuf.end();
        vkCommandBuffers[commandBuffersCount++] = cmdBuf.getHandle();
        stats.commandCount++;
    }
#endif

    VkSemaphore waitSemaphores[MAX_SEMAPHORES_PER_SUBMIT];
    for (uint32 idx = 0; idx < semaphoresToWaitForCount; ++idx) {
        waitSemaphores[idx] = semaphoresToWaitFor[idx]->getHandle();
    }

    VkSemaphore signalSemaphores[MAX_SEMAPHORES_PER_SUBMIT];
    for (uint32 idx = 0; idx < semaphoresToSignalCount; ++idx) {
        signalSemaphores[idx] = semaphoresToSignal[idx]->getHandle();
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = semaphoresToWaitForCount;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBuffersCount;
    submitInfo.pCommandBuffers = vkCommandBuffers;
    submitInfo.signalSemaphoreCount = semaphoresToSignalCount;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // The first CommandBuffer will determine the Queue to use.
    const Queue &queue = *device.getQueueContainer().getQueueByFamilyIndex(commandBuffers[0]->getQueueFamilyIndex());
    BZ_ASSERT_VK(
        vkQueueSubmit(queue.getHandle(), 1, &submitInfo, fenceToSignal ? fenceToSignal->getHandle() : VK_NULL_HANDLE));

    stats.commandBufferCount += commandBuffersCount;
}

void GraphicsContext::waitForDevice() {
    BZ_ASSERT_VK(vkDeviceWaitIdle(device.getHandle()));
}

void GraphicsContext::waitForQueue(QueueProperty queueProperty) {
    const Queue &queue = device.getQueueContainer().getQueueByProperty(queueProperty);
    BZ_ASSERT_VK(vkQueueWaitIdle(queue.getHandle()));
}

void GraphicsContext::onWindowResize(const WindowResizedEvent &e) {
    waitForDevice();

    swapchain.recreate();
    cleanupFrameData();
    createFrameData();

    currentFrameIndex = 0;
}

CommandPool &GraphicsContext::getCurrentFrameCommandPool(QueueProperty property) {
    const Queue &queue = device.getQueueContainer().getQueueByProperty(property);
    return frameDatas[currentFrameIndex].commandPoolsByFamily[queue.getFamily().getIndex()];
}

void GraphicsContext::createFrameData() {
    std::set<uint32> familiesInUse = device.getQueueContainer().getFamilyIndexesInUse();

    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        frameDatas[i].imageAvailableSemaphore = Semaphore::create();
        frameDatas[i].renderFinishedSemaphore = Semaphore::create();
        frameDatas[i].renderFinishedFence = Fence::create(true);

        for (uint32 famIdx : familiesInUse) {
            frameDatas[i].commandPoolsByFamily[famIdx].init(device, famIdx);
        }

#ifdef BZ_GRAPHICS_DEBUG
        frameDatas[i].queryPool.init(device, VK_QUERY_TYPE_TIMESTAMP, TIMESTAMP_QUERY_COUNT, 0);
#endif
    }
}

void GraphicsContext::cleanupFrameData() {
    for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        frameDatas[i].imageAvailableSemaphore.reset();
        frameDatas[i].renderFinishedSemaphore.reset();
        frameDatas[i].renderFinishedFence.reset();

        for (auto &pair : frameDatas[i].commandPoolsByFamily) {
            pair.second.destroy();
        }

#ifdef BZ_GRAPHICS_DEBUG
        frameDatas[i].queryPool.destroy();
#endif
    }
}

void GraphicsContext::onImGuiRender(const FrameTiming &frameTiming) {
    stats.frameTimeCpu = frameTiming.deltaTime;
    stats.runningTime = frameTiming.runningTime;

    statsTimeAcumMs += frameTiming.deltaTime.asMillisecondsUint32();
    if (statsTimeAcumMs >= statsRefreshPeriodMs) {
        statsTimeAcumMs = 0;
        visibleStats = stats;
        frameTimeHistory[frameTimeHistoryIdx] = stats.frameTimeCpu.asMillisecondsFloat();
        frameTimeHistoryIdx = (frameTimeHistoryIdx + 1) % FRAME_HISTORY_SIZE;
    }

    if (ImGui::Begin("Graphics")) {
        ImGui::Text("CPU:");
        ImGui::Text("Frame Time: %.3f ms.", visibleStats.frameTimeCpu.asMillisecondsFloat());
        ImGui::Text("FPS: %.3f.", 1.0f / visibleStats.frameTimeCpu.asSeconds());
        ImGui::Text("Avg Frame Time: %.3f ms.",
                    visibleStats.runningTime.asMillisecondsFloat() / static_cast<float>(visibleStats.frameCount));
        ImGui::Text("Avg FPS: %.3f.",
                    static_cast<float>(visibleStats.frameCount) / visibleStats.runningTime.asSeconds());
        ImGui::Text("Frame Count: %d.", visibleStats.frameCount);
        ImGui::Text("Running Time: %.3f seconds.", visibleStats.runningTime.asSeconds());
        ImGui::Separator();

        ImGui::Text("GPU:");
        ImGui::Text("Frame Time: %.3f ms.", visibleStats.frameTimeGpu.asMillisecondsFloat());
        ImGui::Text("FPS: %.3f.", 1.0f / visibleStats.frameTimeGpu.asSeconds());
        ImGui::Separator();

        ImGui::Text("App is %s bound.", visibleStats.cpuBound ? "CPU" : "GPU");
        ImGui::Separator();

        ImGui::Text("Stats:");
        ImGui::Text("CommandBuffer Count: %d.", visibleStats.commandBufferCount);
        ImGui::Text("Command Count: %d.", visibleStats.commandCount);
        ImGui::Separator();

        ImGui::Text("CPU Frame Times");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.95f);
        ImGui::PlotLines("##plot", frameTimeHistory, FRAME_HISTORY_SIZE, frameTimeHistoryIdx, "ms", 0.0f, 20.0f,
                         ImVec2(0, 80));
        ImGui::Separator();

        ImGui::Text("Refresh period ms");
        ImGui::SliderInt("##slider", reinterpret_cast<int *>(&statsRefreshPeriodMs), 0, 1000);
    }
    ImGui::End();

    stats.reset();
}

#ifdef BZ_GRAPHICS_DEBUG
void GraphicsContext::setObjectDebugName(uint64 handle, VkObjectType objectType, const char *name) {
    VkDebugUtilsObjectNameInfoEXT nameStruct = {};
    nameStruct.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameStruct.objectType = objectType;
    nameStruct.objectHandle = handle;
    nameStruct.pObjectName = name;

    static auto func = getExtensionFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT");
    BZ_ASSERT_VK(func(device.getHandle(), &nameStruct));
}
#endif
}