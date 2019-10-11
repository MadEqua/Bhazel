#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanQueue.h"
#include "Bhazel/Renderer/Framebuffer.h"
#include "Bhazel/Renderer/PipelineState.h"
#include "Bhazel/Renderer/DescriptorSet.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/CommandBuffer.h"


struct GLFWwindow;

namespace BZ {

    class VulkanCommandPool;
    class VulkanDescriptorPool;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;
        void onWindowResize(WindowResizedEvent &e) override;
        void presentBuffer() override;

        void setVSync(bool enabled) override;

        Ref<Framebuffer> getCurrentFrameFramebuffer() override;

        VkDevice getDevice() const { return device; }
        VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

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
        VkDevice device;

        VkPhysicalDevice physicalDevice;

        ///Contains the families of the queues of the selected Device.
        QueueFamilyContainer queueFamilyContainer;

        //The handles may point to the same queues with no restrictions.
        struct QueueContainer {
            VulkanQueue graphics;
            VulkanQueue compute;
            VulkanQueue transfer;
            VulkanQueue present;
        };
        QueueContainer queueContainer;

        //Queues with a single property, if existent (eg: transfer queue).
        //If not existent the handles will refer non-exclusive queues.
        QueueContainer queueContainerExclusive;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<Ref<Framebuffer>> swapchainFramebuffers;
        uint32 swapchainCurrentImageIndex;

        Ref<VulkanDescriptorPool> descriptorPool;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        void createInstance();
        void createSurface();
        void createLogicalDevice(const std::vector<const char*> &requiredDeviceExtensions);
        void createSwapChain();
        void createFramebuffers();
        void createFrameData();
        void createDescriptorPool();

        void recreateSwapChain();
        void cleanupSwapChain();
        void cleanupFramebuffers();
        void cleanupFrameData();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        VkPhysicalDevice pickPhysicalDevice(const std::vector<const char*> &requiredDeviceExtensions);
        bool isPhysicalDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;

        QueueFamilyContainer getQueueFamilies(VkPhysicalDevice device) const;

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
        
        void assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const;

        friend class VulkanRendererApi;
    };
}