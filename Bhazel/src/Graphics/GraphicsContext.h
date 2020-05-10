#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/Internal/Instance.h"
#include "Graphics/Internal/Device.h"
#include "Graphics/Internal/Swapchain.h"
#include "Graphics/Internal/Surface.h"
#include "Graphics/Internal/Sync.h"
#include "Graphics/Internal/CommandPool.h"
#include "Graphics/Internal/DescriptorPool.h"


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
        Semaphore imageAvailableSemaphore;
        Semaphore renderFinishedSemaphore;

        //CPU-GPU sync, to stop the CPU from piling up more than MAX_FRAMES_IN_FLIGHT frames when the app is GPU-bound.
        Fence renderFinishedFence;
    };

    class GraphicsContext {
    public:
        GraphicsContext() = default;

        BZ_NON_COPYABLE(GraphicsContext);
        
        void init();
        void destroy();

        void beginFrame();
        void endFrame();

        void submitCommandBuffers(const CommandBuffer commandBuffers[], uint32 count);
        
        void waitForDevice();

        void onWindowResize(const WindowResizedEvent& e);
        void onImGuiRender(const FrameStats &frameStats); //For statistics.

        const Ref<TextureView>& getColorTextureView() const { return colorTextureView; }
        const Ref<TextureView>& getDepthTextureView() const { return depthTextureView; }
        const Ref<RenderPass>& getMainRenderPass() const { return mainRenderPass; }
        const Ref<Framebuffer>& getMainFramebuffer() const { return mainFramebuffer; }

        CommandPool& getCurrentFrameCommandPool(QueueProperty property, bool exclusive);
        DescriptorPool& getDescriptorPool() { return descriptorPool; }
        VmaAllocator getMemoryAllocator() const { return memoryAllocator; }

        uint32 getCurrentFrameIndex() const { return currentFrameIndex; }

        const Ref<Framebuffer>& getSwapchainAquiredImageFramebuffer() const { return swapchain.getAquiredImageFramebuffer(); }
        const Ref<RenderPass>& getSwapchainRenderPass() const { return swapchain.getRenderPass(); }

        Device& getDevice() { return device; }

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

        //The main framebuffer stuff.
        Ref<TextureView> colorTextureView;
        Ref<TextureView> depthTextureView;
        Ref<RenderPass> mainRenderPass;
        Ref<Framebuffer> mainFramebuffer;

        //CommandBuffers to be submitted at frame end.
        CommandBuffer pendingCommandBuffers[MAX_COMMAND_BUFFERS_PER_FRAME];
        uint32 pendingCommandBufferIndex;

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