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

        const Ref<Framebuffer>& getFramebuffer(int frameIndex) const { return framebuffers[frameIndex]; }
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
        uint32 currentImageIndex = 0;

        void internalInit();
        void createFramebuffers();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };
}