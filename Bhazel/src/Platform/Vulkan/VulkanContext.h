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

#include "Graphics/CommandBuffer.h"

#include "Constants.h"

struct GLFWwindow;


namespace BZ {

    class Framebuffer;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;
        void onWindowResize(const WindowResizedEvent &e) override;

        void setVSync(bool enabled) override;

        uint32 getCurrentFrameIndex() const override { return currentFrameIndex; }
        const Ref<Framebuffer>& getCurrentSwapchainFramebuffer() const override { return swapchain.getCurrentFramebuffer(); }
        const Ref<RenderPass>& getSwapchainRenderPass() const override { return swapchain.getRenderPass(); }
        Ref<CommandBuffer> getCurrentFrameCommandBuffer() override;

        VulkanDevice& getDevice() { return device; }
        //VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        VulkanCommandPool& getCurrentFrameCommandPool(QueueProperty property, bool exclusive);
        VulkanDescriptorPool& getDescriptorPool() { return descriptorPool; }
        VmaAllocator getMemoryAllocator() const { return memoryAllocator; }

        void beginFrame() override;
        void submitCommandBuffersAndFlush(const Ref<CommandBuffer> commandBuffers[], uint32 count) override;
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

            //GPU-GPU sync.
            VulkanSemaphore imageAvailableSemaphore;
            VulkanSemaphore renderFinishedSemaphore;

            //CPU-GPU sync, to stop the CPU from piling up more than MAX_FRAMES_IN_FLIGHT frames when the app is GPU-bound.
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