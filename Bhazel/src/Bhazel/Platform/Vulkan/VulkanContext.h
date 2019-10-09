#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Renderer/Framebuffer.h"
#include "Bhazel/Renderer/PipelineState.h"
#include "Bhazel/Renderer/DescriptorSet.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/CommandBuffer.h"

#include "Bhazel/Core/Timer.h"


struct GLFWwindow;

namespace BZ {

    struct RenderQueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily && computeFamily && transferFamily && presentFamily;
        }
    };

    class VulkanCommandPool;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;

        void onWindowResize(WindowResizedEvent& e) override;

        void presentBuffer() override;

        void setVSync(bool enabled) override;

        Ref<Framebuffer> getCurrentFrameFramebuffer() override;

        VkDevice getDevice() const { return device; }
        VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        const RenderQueueFamilyIndices& getQueueFamilyIndices() { return queueFamilyIndices; }

        uint32 getCurrentFrame() const { return currentFrame; }
        VulkanCommandPool& getCommandPool(RenderQueueFamily family, uint32 frame);

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        struct FrameData {
            //One command pool per frame makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
            Ref<VulkanCommandPool> commandPools[static_cast<int>(RenderQueueFamily::Count)];
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
        RenderQueueFamilyIndices queueFamilyIndices;

        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkQueue transferQueue;
        VkQueue computeQueue;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<Ref<Framebuffer>> swapchainFramebuffers;
        uint32 swapchainCurrentImageIndex;

        Ref<DescriptorPool> descriptorPool;

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

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        VkPhysicalDevice pickPhysicalDevice(const std::vector<const char*> &requiredDeviceExtensions);
        bool isPhysicalDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;

        RenderQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

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