#pragma once

#include <vk_mem_alloc.h>

#include "Graphics/GraphicsContext.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"
#include "Platform/Vulkan/Internal/VulkanSwapchain.h"
#include "Platform/Vulkan/Internal/VulkanSurface.h"
#include "Platform/Vulkan/Internal/VulkanSync.h"
#include "Platform/Vulkan/Internal/VulkanCommandPool.h"
#include "Platform/Vulkan/Internal/VulkanDescriptorPool.h"

#include "Constants.h"

struct GLFWwindow;


namespace BZ {

    class Framebuffer;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;
        void onWindowResize(WindowResizedEvent &e) override;

        void setVSync(bool enabled) override;

        uint32 getCurrentFrameIndex() const override { return currentFrameIndex; }
        Ref<Framebuffer> getCurrentFrameFramebuffer() override { return swapchain.getFramebuffer(currentFrameIndex); }

        VulkanDevice& getDevice() { return device; }
        //VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        VulkanCommandPool& getCurrentFrameCommandPool(QueueProperty property, bool exclusive);
        VulkanDescriptorPool& getDescriptorPool() { return descriptorPool; }
        VmaAllocator getMemoryAllocator() const { return memoryAllocator; }


        /////////////////////////////////////////////////////////
        // API
        /////////////////////////////////////////////////////////
        Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer) override;

        void clearColorAttachments(const Ref<CommandBuffer> &commandBuffer, const Ref<Framebuffer> &framebuffer, const ClearValues &clearColor) override;
        void clearDepthStencilAttachments(const Ref<CommandBuffer> &commandBuffer, const Ref<Framebuffer> &framebuffer, const ClearValues &clearValue) override;

        void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) override;
        void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) override;

        void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) override;
        //void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet) override;
        void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet,
                               const Ref<PipelineState> &pipelineState, uint32 setIndex,
                               uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) override;

        void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) override;
        void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;

        void setViewports(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount) override;
        void setScissorRects(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount) override;

        void endRecording(const Ref<CommandBuffer> &commandBuffer) override;

        void submitCommandBuffersAndFlush(const std::vector<Ref<CommandBuffer>> &pendingCommandBuffers) override;

        void waitForDevice() override;

    private:
        VkInstance instance;
        GLFWwindow *windowHandle;

        VulkanPhysicalDevice physicalDevice;
        VulkanDevice device;
        VulkanSurface surface;
        VulkanSwapchain swapchain;

        struct FrameData {
            //One command pool per frame makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
            std::unordered_map<uint32, VulkanCommandPool> commandPoolsByFamily;
            VulkanSemaphore imageAvailableSemaphore;
            VulkanSemaphore renderFinishedSemaphore;
            VulkanFence renderFinishedFence;
        };
        FrameData frameDatas[MAX_FRAMES_IN_FLIGHT];
        uint32 currentFrameIndex = 0;

        VulkanDescriptorPool descriptorPool;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        VmaAllocator memoryAllocator;

        void createInstance();

        void createFrameData();
        void cleanupFrameData();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        void assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const;
    };
}