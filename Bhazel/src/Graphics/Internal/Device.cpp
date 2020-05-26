#include "bzpch.h"

#include "Device.h"

#include "Graphics/Internal/Surface.h"
#include "Graphics/Internal/Instance.h"


namespace BZ {

    void PhysicalDevice::init(const Instance &instance, const Surface &surface, const std::vector<const char *> &requiredDeviceExtensions) {
        uint32_t deviceCount = 0;
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance.getHandle(), &deviceCount, nullptr));

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance.getHandle(), &deviceCount, physicalDevices.data()));
        for(const auto &physicalDevice : physicalDevices) {
            queueFamilyContainer = getQueueFamilies(physicalDevice, surface.getHandle());
            swapChainSupportDetails = querySwapChainSupport(physicalDevice, surface.getHandle());

            if(isPhysicalDeviceSuitable(physicalDevice, swapChainSupportDetails, queueFamilyContainer, requiredDeviceExtensions)) {
                handle = physicalDevice;
                break;
            }
        }

        BZ_ASSERT_CORE(handle != VK_NULL_HANDLE, "Couldn't find a suitable physical device!");

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(handle, &physicalDeviceProperties);
        BZ_LOG_CORE_INFO("Vulkan PhysicalDevice selected:");
        BZ_LOG_CORE_INFO("  Device Name: {}.", physicalDeviceProperties.deviceName);
        BZ_LOG_CORE_INFO("  Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion), VK_VERSION_MINOR(physicalDeviceProperties.apiVersion), VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
        BZ_LOG_CORE_INFO("  Driver Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion), VK_VERSION_MINOR(physicalDeviceProperties.driverVersion), VK_VERSION_PATCH(physicalDeviceProperties.driverVersion));
        BZ_LOG_CORE_INFO("  VendorId: 0x{:04x}.", physicalDeviceProperties.vendorID);
        BZ_LOG_CORE_INFO("  DeviceId: 0x{:04x}.", physicalDeviceProperties.deviceID);

        limits = physicalDeviceProperties.limits;
    }

    QueueFamilyContainer PhysicalDevice::getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyContainer queueFamilyContainer;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        uint32 idx = 0;
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

                queueFamilyContainer.addFamily({ idx, vkQueueFamilyProps.queueCount, properties, vkQueueFamilyProps.timestampValidBits });
            }
            idx++;
        }

        return queueFamilyContainer;
    }

    SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities));

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

    bool PhysicalDevice::isPhysicalDeviceSuitable(VkPhysicalDevice device, const SwapChainSupportDetails &swapChainSupportDetails, const QueueFamilyContainer &queueFamilyContainer, const std::vector<const char *> &requiredExtensions) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        BZ_ASSERT_CORE(deviceFeatures.geometryShader == VK_TRUE, "Support for geometryShader is assumed!");
        BZ_ASSERT_CORE(deviceFeatures.depthClamp == VK_TRUE, "Support for depthClamp is assumed!");
        BZ_ASSERT_CORE(deviceFeatures.depthBiasClamp == VK_TRUE, "Support for depthBiasClamp is assumed!");
        BZ_ASSERT_CORE(deviceFeatures.samplerAnisotropy == VK_TRUE, "Support for samplerAnisotropy is assumed!");

        bool hasRequiredExtensions = checkDeviceExtensionSupport(device, requiredExtensions);

        bool isSwapChainAdequate = false;
        if(hasRequiredExtensions) {
            isSwapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty(); //TODO: have some requirements
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            queueFamilyContainer.hasAllProperties() &&
            hasRequiredExtensions && isSwapChainAdequate;
    }

    bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions) {
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


    /*-------------------------------------------------------------------------------------------*/
    void Device::init(const PhysicalDevice &physicalDevice, const std::vector<const char *> &requiredDeviceExtensions) {
        this->physicalDevice = &physicalDevice;

        constexpr int QUEUE_PROPS_COUNT = static_cast<int>(QueueProperty::Count);
        int maxScores[QUEUE_PROPS_COUNT] = {};
        const QueueFamily *selectedFamilies[QUEUE_PROPS_COUNT];

        for(const QueueFamily &fam : physicalDevice.getQueueFamilyContainer()) {
            int scores[QUEUE_PROPS_COUNT] = {};

            constexpr int GRAPHICS = static_cast<int>(QueueProperty::Graphics);
            if(fam.hasProperty(QueueProperty::Present))
                scores[GRAPHICS] += 5;
            if(fam.hasProperty(QueueProperty::Compute))
                scores[GRAPHICS] += 1;
            if(fam.hasProperty(QueueProperty::Transfer))
                scores[GRAPHICS] += 1;

            if(scores[GRAPHICS] >= maxScores[GRAPHICS]) {
                maxScores[GRAPHICS] = scores[GRAPHICS];
                selectedFamilies[GRAPHICS] = &fam;
            }

            constexpr int COMPUTE = static_cast<int>(QueueProperty::Compute);
            if(fam.hasProperty(QueueProperty::Present))
                scores[COMPUTE] += 5;
            if(fam.hasProperty(QueueProperty::Graphics))
                scores[COMPUTE] += 1;
            if(fam.hasProperty(QueueProperty::Transfer))
                scores[COMPUTE] += 1;

            if(scores[COMPUTE] >= maxScores[COMPUTE]) {
                maxScores[COMPUTE] = scores[COMPUTE];
                selectedFamilies[COMPUTE] = &fam;
            }

            constexpr int TRANSFER = static_cast<int>(QueueProperty::Transfer);
            if(!fam.hasProperty(QueueProperty::Present))
                scores[TRANSFER] += 1;
            if(!fam.hasProperty(QueueProperty::Graphics))
                scores[TRANSFER] += 1;
            if(!fam.hasProperty(QueueProperty::Compute))
                scores[TRANSFER] += 1;

            if(scores[TRANSFER] >= maxScores[TRANSFER]) {
                maxScores[TRANSFER] = scores[TRANSFER];
                selectedFamilies[TRANSFER] = &fam;
            }

            constexpr int PRESENT = static_cast<int>(QueueProperty::Present);
            if(fam.hasProperty(QueueProperty::Graphics))
                scores[PRESENT] += 5;
            if(fam.hasProperty(QueueProperty::Compute))
                scores[PRESENT] += 4;
            if(fam.hasProperty(QueueProperty::Transfer))
                scores[PRESENT] += 1;

            if(scores[PRESENT] >= maxScores[PRESENT]) {
                maxScores[PRESENT] = scores[PRESENT];
                selectedFamilies[PRESENT] = &fam;
            }
        }

        std::set<uint32> uniqueQueueFamiliesIndices;
        for(uint32 i = 0; i < QUEUE_PROPS_COUNT; ++i) {
            uniqueQueueFamiliesIndices.insert(selectedFamilies[i]->getIndex());
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        for(uint32_t queueFamilyIdx : uniqueQueueFamiliesIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamilyIdx;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        //Add required features here and also when finding physical device.
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.depthClamp = VK_TRUE;
        deviceFeatures.depthBiasClamp = VK_TRUE;
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
        BZ_ASSERT_VK(vkCreateDevice(physicalDevice.getHandle(), &deviceCreateInfo, nullptr, &handle));

        queueContainer.graphics().init(*this, *selectedFamilies[static_cast<int>(QueueProperty::Graphics)]);
        queueContainer.compute().init(*this, *selectedFamilies[static_cast<int>(QueueProperty::Compute)]);
        queueContainer.transfer().init(*this, *selectedFamilies[static_cast<int>(QueueProperty::Transfer)]);
        queueContainer.present().init(*this, *selectedFamilies[static_cast<int>(QueueProperty::Present)]);
    }

    void Device::destroy() {
        vkDestroyDevice(handle, nullptr);
    }
}