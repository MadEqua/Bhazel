#include "bzpch.h"

#include "VulkanDevice.h"
#include "Platform/Vulkan/Internal/VulkanSurface.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    /*VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance, const VulkanSurface &surface, const std::vector<const char *> &requiredDeviceExtensions) {
        init(instance, surface, requiredDeviceExtensions);
    }*/

    void VulkanPhysicalDevice::init(VkInstance instance, const VulkanSurface &surface, const std::vector<const char *> &requiredDeviceExtensions) {
        BZ_ASSERT_CORE(physicalDevice == VK_NULL_HANDLE, "PhysicalDevice is already inited!");

        uint32_t deviceCount = 0;
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

        std::vector<VkPhysicalDevice> devices(deviceCount);
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
        for(const auto &device : devices) {
            queueFamilyContainer = getQueueFamilies(device, surface.getNativeHandle());
            swapChainSupportDetails = querySwapChainSupport(device, surface.getNativeHandle());

            if(isPhysicalDeviceSuitable(device, swapChainSupportDetails, queueFamilyContainer, requiredDeviceExtensions)) {
                physicalDevice = device;
                break;
            }
        }

        BZ_ASSERT_CORE(physicalDevice != VK_NULL_HANDLE, "Couldn't find a suitable physical device!");

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        BZ_LOG_CORE_INFO("Vulkan PhysicalDevice selected:");
        BZ_LOG_CORE_INFO("  Device Name: {}.", physicalDeviceProperties.deviceName);
        BZ_LOG_CORE_INFO("  Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion), VK_VERSION_MINOR(physicalDeviceProperties.apiVersion), VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
        BZ_LOG_CORE_INFO("  Driver Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion), VK_VERSION_MINOR(physicalDeviceProperties.driverVersion), VK_VERSION_PATCH(physicalDeviceProperties.driverVersion));
        BZ_LOG_CORE_INFO("  VendorId: 0x{:04x}.", physicalDeviceProperties.vendorID);
        BZ_LOG_CORE_INFO("  DeviceId: 0x{:04x}.", physicalDeviceProperties.deviceID);
    }

    QueueFamilyContainer VulkanPhysicalDevice::getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyContainer queueFamilyContainer;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int idx = 0, propsIdx = 0;
        for(const auto &vkQueueFamilyProps : queueFamilies) {
            std::vector<QueueProperty> properties;

            if(vkQueueFamilyProps.queueCount > 0) {
                if(vkQueueFamilyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    properties.push_back(QueueProperty::Graphics);
                if(vkQueueFamilyProps.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    properties.push_back(QueueProperty::Compute);
                if(vkQueueFamilyProps.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    properties.push_back(QueueProperty::Transfer);

                VkBool32 presentSupport = false;
                BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface, &presentSupport));
                if(presentSupport)
                    properties.push_back(QueueProperty::Present);

                queueFamilyContainer.addFamily(QueueFamily(idx, vkQueueFamilyProps.queueCount, properties));
            }
            idx++;
        }

        return queueFamilyContainer;
    }

    SwapChainSupportDetails VulkanPhysicalDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

        uint32_t formatCount;
        BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));
        if(formatCount) {
            details.formats.resize(formatCount);
            BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()));
        }

        uint32_t presentModeCount;
        BZ_ASSERT_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));
        if(presentModeCount) {
            details.presentModes.resize(presentModeCount);
            BZ_ASSERT_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data()));
        }

        return details;
    }

    bool VulkanPhysicalDevice::isPhysicalDeviceSuitable(VkPhysicalDevice device, const SwapChainSupportDetails &swapChainSupportDetails, const QueueFamilyContainer &queueFamilyContainer, const std::vector<const char *> &requiredExtensions) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        bool hasRequiredExtensions = checkDeviceExtensionSupport(device, requiredExtensions);

        bool isSwapChainAdequate = false;
        if(hasRequiredExtensions) {
            isSwapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty(); //TODO: have some requirements
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            queueFamilyContainer.hasAllProperties() &&
            hasRequiredExtensions && isSwapChainAdequate;
    }

    bool VulkanPhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions) {
        uint32_t extensionCount;
        BZ_ASSERT_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        BZ_ASSERT_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

        for(const char *extensionName : requiredExtensions) {
            bool extensionFound = false;
            for(const auto &extensionProp : availableExtensions) {
                if(strcmp(extensionName, extensionProp.extensionName) == 0) {
                    extensionFound = true;
                    break;
                }
            }

            if(!extensionFound)
                return false;
        }
        return true;
    }


    /*VulkanDevice::VulkanDevice(const VulkanPhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions) {
        init(physicalDevice, requiredDeviceExtensions);
    }

    VulkanDevice::~VulkanDevice() {
        destroy();
    }*/

    void VulkanDevice::init(const VulkanPhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions) {
        BZ_ASSERT_CORE(device == VK_NULL_HANDLE, "Device is already inited!");

        this->physicalDevice = &physicalDevice;

        auto processFamilies = [this, &physicalDevice](QueueProperty property, const QueueFamily **family, const QueueFamily **familyExclusive) {
            auto families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContain(property);
            for(const QueueFamily *fam : families) {
                if(!*family)
                    *family = fam;
                if(!*familyExclusive && fam->hasExclusiveProperty())
                    *familyExclusive = fam;
            }
        };

        const QueueFamily *graphicsFamily = nullptr;
        const QueueFamily *graphicsFamilyExclusive = nullptr;
        processFamilies(QueueProperty::Graphics, &graphicsFamily, &graphicsFamilyExclusive);

        const QueueFamily *computeFamily = nullptr;
        const QueueFamily *computeFamilyExclusive = nullptr;
        processFamilies(QueueProperty::Compute, &computeFamily, &computeFamilyExclusive);

        const QueueFamily *transferFamily = nullptr;
        const QueueFamily *transferFamilyExclusive = nullptr;
        processFamilies(QueueProperty::Transfer, &transferFamily, &transferFamilyExclusive);

        const QueueFamily *presentFamily = nullptr;
        const QueueFamily *presentFamilyExclusive = nullptr;
        processFamilies(QueueProperty::Present, &presentFamily, &presentFamilyExclusive);


        //We know at this point that at least one of each family exists, so this is safe.
        std::set<uint32_t> uniqueQueueFamiliesIndices = { graphicsFamily->getIndex(),
                                                          computeFamily->getIndex(),
                                                          transferFamily->getIndex(),
                                                          presentFamily->getIndex() };

        if(graphicsFamilyExclusive) uniqueQueueFamiliesIndices.insert(graphicsFamilyExclusive->getIndex());
        if(computeFamilyExclusive) uniqueQueueFamiliesIndices.insert(computeFamilyExclusive->getIndex());
        if(transferFamilyExclusive) uniqueQueueFamiliesIndices.insert(transferFamilyExclusive->getIndex());
        if(presentFamilyExclusive) uniqueQueueFamiliesIndices.insert(presentFamilyExclusive->getIndex());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamiliesIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        //TODO: add required features here and also when finding physical device
        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
        BZ_ASSERT_VK(vkCreateDevice(physicalDevice.getNativeHandle(), &deviceCreateInfo, nullptr, &device));

        queueContainer.graphics.init(*this, *graphicsFamily);
        queueContainer.compute.init(*this, *computeFamily);
        queueContainer.transfer.init(*this, *transferFamily);
        queueContainer.present.init(*this, *presentFamily);

        //If there is no exclusive queue, then fill with the same data as the non-exclusive queue.
        queueContainerExclusive.graphics.init(*this, graphicsFamilyExclusive ? *graphicsFamilyExclusive : *graphicsFamily);
        queueContainerExclusive.compute.init(*this, computeFamilyExclusive ? *computeFamilyExclusive : *computeFamily);
        queueContainerExclusive.transfer.init(*this, transferFamilyExclusive ? *transferFamilyExclusive : *transferFamily);
        queueContainerExclusive.present.init(*this, presentFamilyExclusive ? *presentFamilyExclusive : *presentFamily);
    }

    void VulkanDevice::destroy() {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
}