#pragma once

#include "VulkanIncludes.h"

#include "Graphics/Internal/Queue.h"
#include "Graphics/Internal/Swapchain.h"


namespace BZ {

class Surface;
class Instance;
enum class MemoryType;

// Will pick and store an appropriate physical device to use, if available.
class PhysicalDevice {
  public:
    PhysicalDevice() = default;

    BZ_NON_COPYABLE(PhysicalDevice);

    void init(const Instance &instance, const Surface &surface,
              const std::vector<const char *> &requiredDeviceExtensions);

    const VkPhysicalDeviceLimits &getLimits() const { return limits; }

    const QueueFamilyContainer &getQueueFamilyContainer() const { return queueFamilyContainer; }
    const SwapChainSupportDetails &getSwapChainSupportDetails() const { return swapChainSupportDetails; }
    VkPhysicalDevice getHandle() const { return handle; }

  private:
    VkPhysicalDevice handle = VK_NULL_HANDLE;

    QueueFamilyContainer queueFamilyContainer;
    SwapChainSupportDetails swapChainSupportDetails;

    VkPhysicalDeviceLimits limits;

    static QueueFamilyContainer getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool isPhysicalDeviceSuitable(VkPhysicalDevice device,
                                         const SwapChainSupportDetails &swapChainSupportDetails,
                                         const QueueFamilyContainer &queueFamilyContainer,
                                         const std::vector<const char *> &requiredExtensions);
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device,
                                            const std::vector<const char *> &requiredExtensions);

    friend class Device;
};


/*-------------------------------------------------------------------------------------------*/
// Representing a logical device.
class Device {
  public:
    Device() = default;

    BZ_NON_COPYABLE(Device);

    void init(const PhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions);
    void destroy();

    VkDevice getHandle() const { return handle; }

    const QueueContainer &getQueueContainer() const { return queueContainer; }
    const PhysicalDevice &getPhysicalDevice() const { return *physicalDevice; }

  private:
    VkDevice handle = VK_NULL_HANDLE;
    const PhysicalDevice *physicalDevice;

    QueueContainer queueContainer;
};
}