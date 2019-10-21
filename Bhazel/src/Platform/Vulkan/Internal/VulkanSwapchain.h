#pragma once

#include "VulkanIncludes.h"


namespace BZ {

    class Framebuffer;
    class VulkanDevice;
    class VulkanSurface;
    class VulkanSemaphore;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanSwapchain {
    public:
        VulkanSwapchain() = default;
        //VulkanSwapchain(const VulkanDevice &device, const VulkanSurface &surface);
        //~VulkanSwapchain();

        void init(const VulkanDevice &device, const VulkanSurface &surface);
        void destroy();

        void recreate();

        void aquireImage (const VulkanSemaphore &imageAvailableSemaphore);
        void presentImage(const VulkanSemaphore &renderFinishedSemaphore);

        VkSwapchainKHR getNativeHandle() const { return swapchain; }

        Ref<Framebuffer> getFramebuffer(int frameIndex) { return framebuffers[frameIndex]; }

    private:
        const VulkanDevice *device;
        const VulkanSurface *surface;

        VkSwapchainKHR swapchain;
        std::vector<Ref<Framebuffer>> framebuffers;

        VkFormat imageFormat;
        VkExtent2D extent;
        uint32 currentImageIndex = 0;

        void internalInit();
        void createFramebuffers();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}