#pragma once

#include "Graphics/GraphicsContext.h"

#include "Platform/Vulkan/VulkanIncludes.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"

#include "Graphics/Graphics.h"


struct GLFWwindow;

namespace BZ {

    class VulkanCommandPool;
    class VulkanDescriptorPool;
    class Framebuffer;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;
        void onWindowResize(WindowResizedEvent &e) override;
        void presentBuffer() override;

        void setVSync(bool enabled) override;

        Ref<Framebuffer> getCurrentFrameFramebuffer() override;
        Ref<Framebuffer> getFramebuffer(uint32 frameIdx) override;

        VkDevice getDevice() const { return device.getNativeHandle(); }
        //VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        uint32 getCurrentFrame() const { return currentFrame; }
        VulkanCommandPool& getCommandPool(QueueProperty property, uint32 frame, bool exclusive);
        VulkanDescriptorPool& getDescriptorPool() { return *descriptorPool; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        struct FrameData {
            //One command pool per frame makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
            std::unordered_map<uint32, Ref<VulkanCommandPool>> commandPoolsByFamily;
            VkSemaphore imageAvailableSemaphore;
            VkSemaphore renderFinishedSemaphore;
            VkFence inFlightFence;
        };
        FrameData frameData[MAX_FRAMES_IN_FLIGHT];
        uint32 currentFrame = 0;

        GLFWwindow *windowHandle;

        VkInstance instance;
        VkSurfaceKHR surface;

        VulkanPhysicalDevice physicalDevice;
        VulkanDevice device;
        VulkanSwapchain swapchain;

        Ref<VulkanDescriptorPool> descriptorPool;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        void createInstance();
        void createSurface();
        void createDevice();
        void createSwapchain();
        void createFrameData();
        void createDescriptorPool();

        void cleanupFrameData();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        void assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const;

        friend class VulkanGraphicsApi;
    };
}