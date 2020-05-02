#pragma once

#include "VulkanIncludes.h"


namespace BZ {

    class Framebuffer;
    class RenderPass;
    class VulkanDevice;
    class VulkanSurface;
    class VulkanSemaphore;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /*
    * Abstract the Swapchain and respective Framebuffer creation, also manages the Depth Buffer texture for convenience.
    * Only has one image aquired at any given time.
    */
    class VulkanSwapchain {
    public:
        VulkanSwapchain() = default;

        void init(const VulkanDevice &device, const VulkanSurface &surface);
        void destroy();

        void recreate();

        void aquireImage(const VulkanSemaphore &imageAvailableSemaphore);
        void presentImage(const VulkanSemaphore &renderFinishedSemaphore);

        VkSwapchainKHR getNativeHandle() const { return swapchain; }

        const Ref<Framebuffer>& getCurrentFramebuffer() const { return framebuffers[currentImageIndex]; }
        const Ref<RenderPass>& getRenderPass() const { return renderPass; }
        glm::ivec2 getDimensions() const { return { extent.width, extent.height }; }

    private:
        const VulkanDevice *device;
        const VulkanSurface *surface;

        VkSwapchainKHR swapchain;

        Ref<RenderPass> renderPass;
        std::vector<Ref<Framebuffer>> framebuffers;

        VkFormat imageFormat;
        VkExtent2D extent;

        //This is OK as long as we maintain a steady aquire -> present cycle, ie, only have one image aquired at a time.
        uint32 currentImageIndex = 0;
        bool aquired = false;

        void internalInit();
        void createFramebuffers();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}