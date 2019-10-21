#pragma once

#include "VulkanIncludes.h"

#include "Platform/Vulkan/Internal/VulkanQueue.h"
#include "Platform/Vulkan/Internal/VulkanSwapchain.h"


namespace BZ {
    
    class VulkanSurface;


    //Will pick and store an appropriate physical device to use, if available.
    class VulkanPhysicalDevice {
    public:
        VulkanPhysicalDevice() = default;
        //VulkanPhysicalDevice(VkInstance instance, const VulkanSurface &surface, const std::vector<const char *> &requiredDeviceExtensions);

        void init(VkInstance instance, const VulkanSurface &surface, const std::vector<const char *> &requiredDeviceExtensions);

        const QueueFamilyContainer& getQueueFamilyContainer() const { return queueFamilyContainer; }
        const SwapChainSupportDetails& getSwapChainSupportDetails() const { return swapChainSupportDetails; }
        VkPhysicalDevice getNativeHandle() const { return physicalDevice; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        QueueFamilyContainer queueFamilyContainer;
        SwapChainSupportDetails swapChainSupportDetails;

        static QueueFamilyContainer getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool isPhysicalDeviceSuitable(VkPhysicalDevice device, const SwapChainSupportDetails &swapChainSupportDetails, const QueueFamilyContainer &queueFamilyContainer, const std::vector<const char *> &requiredExtensions);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions);

        friend class VulkanDevice;
    };


    //Representing a logical device.
    class VulkanDevice {
    public:
        VulkanDevice() = default;
        //VulkanDevice(const VulkanPhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions);
        //~VulkanDevice();

        void init(const VulkanPhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions);
        void destroy();

        VkDevice getNativeHandle() const { return device; }

        const QueueContainer &getQueueContainer() const { return queueContainer; }
        const QueueContainer &getQueueContainerExclusive() const { return queueContainerExclusive; }
        const VulkanPhysicalDevice &getPhysicalDevice() const { return *physicalDevice; }

    private:
        const VulkanPhysicalDevice *physicalDevice = nullptr;

        VkDevice device = VK_NULL_HANDLE;

        QueueContainer queueContainer;

        //Queues with a single property, if existent (eg: transfer queue).
        //If not existent the handles will refer non-exclusive queues.
        QueueContainer queueContainerExclusive;
    };
}