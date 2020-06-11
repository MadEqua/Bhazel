#pragma once

#include "Graphics/Internal/CommandPool.h"
#include "Graphics/Internal/DescriptorPool.h"
#include "Graphics/Internal/Device.h"
#include "Graphics/Internal/Instance.h"
#include "Graphics/Internal/QueryPool.h"
#include "Graphics/Internal/Surface.h"
#include "Graphics/Internal/Swapchain.h"
#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/Sync.h"


namespace BZ {

class WindowResizedEvent;
class Framebuffer;
class RenderPass;
class CommandBuffer;
class TextureView;
struct FrameTiming;
class Window;


class GraphicsContext {
  public:
    GraphicsContext() = default;

    BZ_NON_COPYABLE(GraphicsContext);

    void init();
    void destroy();

    void beginFrame();
    void endFrame();

    void submitCommandBuffers(const CommandBuffer *commandBuffers[], uint32 commandBuffersCount,
                              bool waitForImageAvailable, bool signalFrameEnd);

    void submitCommandBuffers(const CommandBuffer *commandBuffers[], uint32 commandBuffersCount,
                              const Ref<Semaphore> semaphoresToWaitFor[], VkPipelineStageFlags waitStages[],
                              uint32 semaphoresToWaitForCount, const Ref<Semaphore> semaphoresToSignal[],
                              uint32 semaphoresToSignalCount, const Ref<Fence> &fenceToSignal);

    void waitForDevice();
    void waitForQueue(QueueProperty queueProperty);

    void onWindowResize(const WindowResizedEvent &e);
    void onImGuiRender(const FrameTiming &frameTiming); // For statistics.

    CommandPool &getCurrentFrameCommandPool(QueueProperty property);
    DescriptorPool &getDescriptorPool() { return descriptorPool; }
    VmaAllocator getMemoryAllocator() const { return memoryAllocator; }

    uint32 getCurrentFrameIndex() const { return currentFrameIndex; }

    const Ref<Framebuffer> &getSwapchainAquiredImageFramebuffer() const {
        return swapchain.getAquiredImageFramebuffer();
    }
    const Ref<RenderPass> &getSwapchainRenderPass() const { return swapchain.getRenderPass(); }

    const Ref<Semaphore> &getCurrentFrameRenderFinishedSemaphore() const {
        return frameDatas[currentFrameIndex].renderFinishedSemaphore;
    }
    const Ref<Semaphore> &getCurrentFrameImageAvailableSemaphore() const {
        return frameDatas[currentFrameIndex].imageAvailableSemaphore;
    }
    const Ref<Fence> &getCurrentFrameRenderFinishedFence() const {
        return frameDatas[currentFrameIndex].renderFinishedFence;
    }

#ifdef BZ_GRAPHICS_DEBUG
    void setObjectDebugName(uint64 handle, VkObjectType objectType, const char *name);
#endif

    Device &getDevice() { return device; }

    template <typename T> T getExtensionFunction(const char *name) { return instance.getExtensionFunction<T>(name); }

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

    VmaAllocator memoryAllocator;

    struct FrameData {
        // One command pool per frame (per family) makes it easy to reset all the allocated buffers on frame end. No
        // need to track anything else.
        std::unordered_map<uint32, CommandPool> commandPoolsByFamily;

        // GPU-GPU sync.
        Ref<Semaphore> imageAvailableSemaphore;
        Ref<Semaphore> renderFinishedSemaphore;

        // CPU-GPU sync, to stop the CPU from piling up more than MAX_FRAMES_IN_FLIGHT frames when the app is GPU-bound.
        Ref<Fence> renderFinishedFence;

#ifdef BZ_GRAPHICS_DEBUG
        // For timestamp queries.
        QueryPool queryPool;
#endif
    };

    FrameData frameDatas[MAX_FRAMES_IN_FLIGHT];
    uint32 currentFrameIndex = 0;


    // Statistics stuff.
    struct GraphicsStats {
        // Time spent by the CPU updating and generating the CommandBuffers for the last frame.
        TimeDuration frameTimeCpu;

        // Time spent by the GPU running the all the generated CommandBuffers.
        // Not for the last frame, there's a delay of MAX_FRAMES_IN_FLIGHT frames.
        TimeDuration frameTimeGpu;

        uint64 frameCount;
        TimeDuration runningTime;

        uint32 commandCount;
        uint32 commandBufferCount;

        // Are we CPU bound? If not, we are GPU bound.
        bool cpuBound;

        void reset() {
            uint64 frameCount = this->frameCount;
            memset(this, 0, sizeof(GraphicsStats));
            this->frameCount = frameCount;
        }
    };

    GraphicsStats stats;
    GraphicsStats visibleStats;

    uint32 statsRefreshPeriodMs = 250;
    uint32 statsTimeAcumMs;

    constexpr static int FRAME_HISTORY_SIZE = 100;
    float frameTimeHistory[FRAME_HISTORY_SIZE] = {};
    uint32 frameTimeHistoryIdx = 0;

    constexpr static uint32 TIMESTAMP_QUERY_COUNT = 2;
};


#ifdef BZ_GRAPHICS_DEBUG
    #define BZ_SET_BUFFER_DEBUG_NAME(buffer, name)                                                     \
        BZ_GRAPHICS_CTX.setObjectDebugName(reinterpret_cast<uint64>(buffer->getHandle().bufferHandle), \
                                           VK_OBJECT_TYPE_BUFFER, name);
    #define BZ_SET_FRAMEBUFFER_DEBUG_NAME(fb, name) \
        BZ_GRAPHICS_CTX.setObjectDebugName(reinterpret_cast<uint64>(fb->getHandle()), VK_OBJECT_TYPE_FRAMEBUFFER, name);
    #define BZ_SET_PIPELINE_DEBUG_NAME(pipeline, name)                                                               \
        BZ_GRAPHICS_CTX.setObjectDebugName(reinterpret_cast<uint64>(pipeline->getHandle()), VK_OBJECT_TYPE_PIPELINE, \
                                           name);
    #define BZ_SET_TEXTURE_DEBUG_NAME(tex, name)                                                   \
        BZ_GRAPHICS_CTX.setObjectDebugName(reinterpret_cast<uint64>(tex->getHandle().imageHandle), \
                                           VK_OBJECT_TYPE_IMAGE, name);
#else
    #define BZ_SET_BUFFER_DEBUG_NAME(buffer, name)
    #define BZ_SET_FRAMEBUFFER_DEBUG_NAME(fb, name)
    #define BZ_SET_PIPELINE_DEBUG_NAME(pipeline, name)
    #define BZ_SET_TEXTURE_DEBUG_NAME(tex, name)
#endif
}