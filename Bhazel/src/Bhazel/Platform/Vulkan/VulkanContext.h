#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Renderer/Framebuffer.h"
#include "Bhazel/Renderer/PipelineState.h"


struct GLFWwindow;

namespace BZ {

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;

        void onWindowResize(WindowResizedEvent& e) override;
        void presentBuffer() override;

        void setVSync(bool enabled) override;

        VkDevice getDevice() const { return device; }
        VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() const {
                return graphicsFamily && presentFamily;
            }
        };

        constexpr static int MAX_FRAMES_IN_FLIGHT = 2;

        GLFWwindow *windowHandle;

        VkInstance instance;
        VkDevice device;

        VkPhysicalDevice physicalDevice;
        QueueFamilyIndices queueFamilyIndices;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        //std::vector<Ref<Texture>> swapChainTextures;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        //std::array<Ref<TextureView>, MAX_FRAMES_IN_FLIGHT> swapChainTextureViews;
        std::vector<Ref<Framebuffer>> swapChainFramebuffers;


        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;

        size_t currentFrame = 0;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        void createInstance();
        void createSurface();
        void createLogicalDevice(const std::vector<const char*> &requiredDeviceExtensions);
        void createSwapChain();
        void createFramebuffers();
        void createSyncObjects();

        void recreateSwapChain();
        void cleanupSwapChain();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        VkPhysicalDevice pickPhysicalDevice(const std::vector<const char*> &requiredDeviceExtensions);
        bool isPhysicalDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

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


        //TODO: temporary test stuff
        void createCommandBuffers();

        void initTestStuff();
        void draw();

        Ref<Buffer> vertexBuffer;
        Ref<Buffer> indexBuffer;
        Ref<Buffer> constantBuffer;
        Ref<PipelineState> pipelineState;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
    };
}