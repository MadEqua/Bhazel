#pragma once

#include "Graphics/GraphicsContext.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"
#include "Platform/Vulkan/Internal/VulkanSwapchain.h"
#include "Platform/Vulkan/Internal/VulkanSurface.h"
#include "Platform/Vulkan/Internal/VulkanSync.h"

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
        VkInstance instance;
        GLFWwindow *windowHandle;

        VulkanPhysicalDevice physicalDevice;
        VulkanDevice device;
        VulkanSurface surface;
        VulkanSwapchain swapchain;

        struct FrameData {
            //One command pool per frame makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
            std::unordered_map<uint32, Ref<VulkanCommandPool>> commandPoolsByFamily;
            VulkanSemaphore imageAvailableSemaphore;
            VulkanSemaphore renderFinishedSemaphore;
            VulkanFence inFlightFence;
        };
        FrameData frameData[MAX_FRAMES_IN_FLIGHT];
        uint32 currentFrame = 0;

        Ref<VulkanDescriptorPool> descriptorPool;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        void createInstance();

        void createFrameData();
        void cleanupFrameData();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        void assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const;

        friend class VulkanGraphicsApi;
    };
}