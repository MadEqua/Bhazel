#pragma once

#include "VulkanIncludes.h"


namespace BZ {

    class Framebuffer;
    class VulkanDevice;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanSwapchain {
    public:
        VulkanSwapchain() = default;
        //VulkanSwapchain(const VulkanDevice &device, VkSurfaceKHR surface);
        ~VulkanSwapchain();

        void init(const VulkanDevice &device, VkSurfaceKHR surface);

        void recreate();

        void aquireImage(VkSemaphore imageAvailableSemaphore);
        void presentImage(VkSemaphore renderFinishedSemaphore);

        VkSwapchainKHR getNativeHandle() const { return swapchain; }

        Ref<Framebuffer> getFramebuffer(int frameIndex) { return framebuffers[frameIndex]; }

    private:
        const VulkanDevice *device;
        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain;
        std::vector<Ref<Framebuffer>> framebuffers;

        VkFormat imageFormat;
        VkExtent2D extent;
        uint32 currentImageIndex = 0;

        void init();
        void createFramebuffers();
        void clean();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}