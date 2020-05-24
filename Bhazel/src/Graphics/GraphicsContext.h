#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/Internal/Instance.h"
#include "Graphics/Internal/Device.h"
#include "Graphics/Internal/Swapchain.h"
#include "Graphics/Internal/Surface.h"
#include "Graphics/Internal/CommandPool.h"
#include "Graphics/Internal/DescriptorPool.h"
#include "Graphics/Sync.h"


namespace BZ {

    struct FrameStats {
        TimeDuration lastFrameTime;
        uint64 frameCount;
        TimeDuration runningTime;
    };

    class WindowResizedEvent;
    class Framebuffer;
    class RenderPass;
    class CommandBuffer;
    class TextureView;

    struct FrameData {
        //One command pool per frame (per family) makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
        std::unordered_map<uint32, CommandPool> commandPoolsByFamily;

        //GPU-GPU sync.
        Ref<Semaphore> imageAvailableSemaphore;
        Ref<Semaphore> renderFinishedSemaphore;

        //CPU-GPU sync, to stop the CPU from piling up more than MAX_FRAMES_IN_FLIGHT frames when the app is GPU-bound.
        Ref<Fence> renderFinishedFence;
    };

    class GraphicsContext {
    public:
        GraphicsContext() = default;

        BZ_NON_COPYABLE(GraphicsContext);
        
        void init();
        void destroy();

        void beginFrame();
        void endFrame();

        void submitCommandBuffers(const CommandBuffer* commandBuffers[], uint32 commandBuffersCount,
                                  bool waitForImageAvailable, bool signalFrameEnd);

        void submitCommandBuffers(const CommandBuffer* commandBuffers[], uint32 commandBuffersCount,
                                  const Ref<Semaphore> semaphoresToWaitFor[], VkPipelineStageFlags waitStages[], uint32 semaphoresToWaitForCount,
                                  const Ref<Semaphore> semaphoresToSignal[], uint32 semaphoresToSignalCount,
                                  const Ref<Fence> &fenceToSignal);

        void waitForDevice();
        void waitForQueue(QueueProperty queueProperty);

        void onWindowResize(const WindowResizedEvent& e);
        void onImGuiRender(const FrameStats &frameStats); //For statistics.

        CommandPool& getCurrentFrameCommandPool(QueueProperty property);
        DescriptorPool& getDescriptorPool() { return descriptorPool; }
        VmaAllocator getMemoryAllocator() const { return memoryAllocator; }

        uint32 getCurrentFrameIndex() const { return currentFrameIndex; }

        const Ref<Framebuffer>& getSwapchainAquiredImageFramebuffer() const { return swapchain.getAquiredImageFramebuffer(); }
        const Ref<RenderPass>& getSwapchainRenderPass() const { return swapchain.getRenderPass(); }

        const Ref<Semaphore>& getCurrentFrameRenderFinishedSemaphore() const { return frameDatas[currentFrameIndex].renderFinishedSemaphore; }
        const Ref<Semaphore>& getCurrentFrameImageAvailableSemaphore() const { return frameDatas[currentFrameIndex].imageAvailableSemaphore; }
        const Ref<Fence>& getCurrentFrameRenderFinishedFence() const { return frameDatas[currentFrameIndex].renderFinishedFence; }

        Device& getDevice() { return device; }

        constexpr static uint32 MAX_FRAMES_IN_FLIGHT = 3;
        constexpr static uint32 MAX_COMMAND_BUFFERS_PER_SUBMIT = 32;
        constexpr static uint32 MAX_SEMAPHORES_PER_SUBMIT = 8;
        constexpr static uint32 MIN_UNIFORM_BUFFER_OFFSET_ALIGN = 256;

    private:
        void createFrameData();
        void cleanupFrameData();

        Instance instance;
        Surface surface;
        PhysicalDevice physicalDevice;
        Device device;
        Swapchain swapchain;

        DescriptorPool descriptorPool;

        FrameData frameDatas[MAX_FRAMES_IN_FLIGHT];
        uint32 currentFrameIndex = 0;

        VmaAllocator memoryAllocator;

        //Statistics stuff.
        struct GraphicsStats {
            FrameStats frameStats;
            uint32 commandBufferCount;
            uint32 commandCount;
        };

        GraphicsStats stats;
        GraphicsStats visibleStats;

        uint32 statsRefreshPeriodMs = 250;
        uint32 statsTimeAcumMs;

        constexpr static int FRAME_HISTORY_SIZE = 100;
        float frameTimeHistory[FRAME_HISTORY_SIZE] = {};
        uint32 frameTimeHistoryIdx = 0;
    };
}