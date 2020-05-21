#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Framebuffer;
    class RenderPass;
    class Device;
    class Surface;
    class Semaphore;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /*
    * Abstract the Swapchain and respective Framebuffer creation. The RenderPass and Framebuffers reutilize the main (and only) DepthBuffer.
    * Only has one image aquired at any given time.
    */
    class Swapchain {
    public:
        Swapchain() = default;

        BZ_NON_COPYABLE(Swapchain);

        void init(const Device &device, const Surface &surface);
        void destroy();

        void recreate();

        void aquireImage(const Ref<Semaphore> &imageAvailableSemaphore);
        void presentImage(const Ref<Semaphore> &renderFinishedSemaphore);

        VkSwapchainKHR getHandle() const { return handle; }

        const Ref<Framebuffer>& getAquiredImageFramebuffer() const { return framebuffers[currentImageIndex]; }
        const Ref<RenderPass>& getDefaultRenderPass() const { return defaultRenderPass; }

        glm::ivec2 getDimensions() const { return { extent.width, extent.height }; }

    private:
        const Device *device;
        const Surface *surface;

        VkSwapchainKHR handle;

        Ref<RenderPass> defaultRenderPass;
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
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
    };
}