#pragma once

#include "Bhazel/Renderer/GraphicsContext.h"

#include "VulkanIncludes.h"


struct GLFWwindow;

namespace BZ {

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(GLFWwindow *windowHandle);
        ~VulkanContext() override;

        void swapBuffers() override;
        void setVSync(bool enabled) override;

    private:
        GLFWwindow *windowHandle;

        VkInstance instance;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        const int MAX_FRAMES_IN_FLIGHT = 2;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        size_t currentFrame = 0;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        bool isPhysicalDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const;

        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() {
                return graphicsFamily && presentFamily;
            }
        };
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
        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkFramebuffer> swapChainFramebuffers;

        void doTestStuff(QueueFamilyIndices &queueFamilyIndices);
        void draw();
        VkShaderModule createShaderModule(const std::vector<char>& code);

        static std::vector<char> readFile(const std::string& filename);
    };
}