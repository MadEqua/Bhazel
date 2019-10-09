#include "bzpch.h"

#include "Bhazel/Platform/Vulkan/VulkanContext.h"
#include "Bhazel/Platform/Vulkan/VulkanRendererAPI.h"
#include "Bhazel/Platform/Vulkan/VulkanTexture.h"
#include "Bhazel/Platform/Vulkan/VulkanPipelineState.h"
#include "Bhazel/Platform/Vulkan/VulkanFramebuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanBuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanConversions.h"
#include "Bhazel/Platform/Vulkan/VulkanDescriptorSet.h"
#include "Bhazel/Platform/Vulkan/VulkanCommandBuffer.h"

#include "Bhazel/Renderer/CommandBuffer.h"
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/Buffer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include <fstream>


namespace BZ {

    VulkanContext::VulkanContext(void *windowHandle) :
        windowHandle(static_cast<GLFWwindow*>(windowHandle)) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null!");
     }

    VulkanContext::~VulkanContext() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device));

        cleanupSwapChain();

        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(device, frameData[i].renderFinishedSemaphore, nullptr);
            vkDestroySemaphore(device, frameData[i].imageAvailableSemaphore, nullptr);
            vkDestroyFence(device, frameData[i].inFlightFence, nullptr);

            for(int fam = 0; fam < static_cast<int>(RenderQueueFamily::Count); ++fam) {
                frameData[i].commandPools[fam].reset();
            }
        }

        vkDestroyDevice(device, nullptr);

        vkDestroySurfaceKHR(instance, surface, nullptr);

#ifndef BZ_DIST
        auto func = getExtensionFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(instance, "vkDestroyDebugUtilsMessengerEXT");
        func(instance, debugMessenger, nullptr);
#endif
        vkDestroyInstance(instance, nullptr);
    }

    void VulkanContext::init() {
        createInstance();
        createSurface();

        const std::vector<const char *> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
        };
        physicalDevice = pickPhysicalDevice(requiredDeviceExtensions);
        BZ_ASSERT_CORE(physicalDevice != VK_NULL_HANDLE, "Couldn't find a suitable physical device!");

        createLogicalDevice(requiredDeviceExtensions);
        createFrameData();

        createSwapChain();
        createFramebuffers();
        createDescriptorPool();

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        BZ_LOG_CORE_INFO("Vulkan Context:");
        BZ_LOG_CORE_INFO("  Device Name: {}.", physicalDeviceProperties.deviceName);
        BZ_LOG_CORE_INFO("  Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion), VK_VERSION_MINOR(physicalDeviceProperties.apiVersion), VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
        BZ_LOG_CORE_INFO("  Driver Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion), VK_VERSION_MINOR(physicalDeviceProperties.driverVersion), VK_VERSION_PATCH(physicalDeviceProperties.driverVersion));
        BZ_LOG_CORE_INFO("  VendorId: 0x{:04x}.", physicalDeviceProperties.vendorID);
        BZ_LOG_CORE_INFO("  DeviceId: 0x{:04x}.", physicalDeviceProperties.deviceID);

        rendererApi = std::make_unique<VulkanRendererApi>(*this);
    }

    void VulkanContext::presentBuffer() {
        VkSemaphore waitSemaphore = { frameData[currentFrame].renderFinishedSemaphore };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &swapchainCurrentImageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to present image. Error: {}.", result);
            recreateSwapChain();
        }

        for(int fam = 0; fam < static_cast<int>(RenderQueueFamily::Count); ++fam) {
            frameData[currentFrame].commandPools[fam]->reset();
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frameData[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &swapchainCurrentImageIndex);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to acquire image for presentation. Error: {}.", result);
            recreateSwapChain();
        }
    }

    void VulkanContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);
    }

    Ref<Framebuffer> VulkanContext::getCurrentFrameFramebuffer() {
        return swapchainFramebuffers[currentFrame];
    }

    VulkanCommandPool& VulkanContext::getCommandPool(RenderQueueFamily family, uint32 frame) {
        BZ_ASSERT_CORE(frame < MAX_FRAMES_IN_FLIGHT, "Invalid frame: {}!", frame);
        return *frameData[frame].commandPools[static_cast<int>(family)];
    }

    uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        BZ_ASSERT_ALWAYS_CORE("Can't find a suitable memory type!");
        return 0;
    }

    void VulkanContext::onWindowResize(WindowResizedEvent& e) {
        recreateSwapChain();
    }

    void VulkanContext::createInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = nullptr;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Bhazel Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //Ask GLFW which extensions it needs for platform specific stuff.
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredInstanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef BZ_DIST

        //Request the debug utils extension. This way we can handle validation layers messages with Bhazel loggers.
        requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //Request a validation layer
        const std::vector<const char*> layersToRequest = {
            "VK_LAYER_KHRONOS_validation"
        };
        assertValidationLayerSupport(layersToRequest);

        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
        createInfo.ppEnabledLayerNames = layersToRequest.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

        BZ_ASSERT_VK(vkCreateInstance(&createInfo, nullptr, &instance));

#ifndef BZ_DIST
        //Set the debug callback function
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = vulkanCallbackFunction;
        debugUtilsCreateInfo.pUserData = nullptr;

        auto func = getExtensionFunction<PFN_vkCreateDebugUtilsMessengerEXT>(instance, "vkCreateDebugUtilsMessengerEXT");
        BZ_ASSERT_VK(func(instance, &debugUtilsCreateInfo, nullptr, &debugMessenger));
#endif
    }

    void VulkanContext::createSurface() {
        BZ_ASSERT_VK(glfwCreateWindowSurface(instance, windowHandle, nullptr, &surface));
    }

    void VulkanContext::createLogicalDevice(const std::vector<const char*> &requiredDeviceExtensions) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), 
                                                  queueFamilyIndices.presentFamily.value(),
                                                  queueFamilyIndices.computeFamily.value(),
                                                  queueFamilyIndices.transferFamily.value() };
        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamilies) {
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
        BZ_ASSERT_VK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

        vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
        vkGetDeviceQueue(device, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
        vkGetDeviceQueue(device, queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
    }

    void VulkanContext::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        uint32_t imageCount = MAX_FRAMES_IN_FLIGHT;//swapChainSupport.capabilities.minImageCount + 1; TODO
        if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = surface;
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndicesArr[] = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};
        if(queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
        }
        else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
            swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        BZ_ASSERT_VK(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapchain));

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;
    }

    void VulkanContext::createFramebuffers() {
        //Get the images created for the swapchain
        std::vector<VkImage> swapChainImages;
        uint32_t imageCount;
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
        swapChainImages.resize(imageCount);
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data()));

        //Create Views for the images
       swapchainFramebuffers.resize(imageCount);
       for(size_t i = 0; i < swapChainImages.size(); i++) {
            auto textureRef = VulkanTexture2D::wrap(swapChainImages[i], swapchainExtent.width, swapchainExtent.height, swapchainImageFormat);
            auto textureViewRef = TextureView::create(textureRef);

            AttachmentDescription attachmentDesc;
            attachmentDesc.format = textureRef->getFormat();
            attachmentDesc.samples = 1;
            attachmentDesc.loadOperatorColorAndDepth = LoadOperation::Clear;
            attachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
            attachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
            attachmentDesc.storeOperatorStencil = StoreOperation::DontCare;
            attachmentDesc.initialLayout = TextureLayout::Undefined;
            attachmentDesc.finalLayout = TextureLayout::Present;

            Framebuffer::Builder builder;
            builder.addColorAttachment(attachmentDesc, textureViewRef);
            builder.setDimensions(glm::ivec3(swapchainExtent.width, swapchainExtent.height, 1));

            swapchainFramebuffers[i] = builder.build();
        }

       BZ_ASSERT_VK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frameData[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &swapchainCurrentImageIndex));
    }

    void VulkanContext::createFrameData() {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            BZ_ASSERT_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameData[i].imageAvailableSemaphore));
            BZ_ASSERT_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameData[i].renderFinishedSemaphore));
            BZ_ASSERT_VK(vkCreateFence(device, &fenceInfo, nullptr, &frameData[i].inFlightFence));

            for(int fam = 0; fam < static_cast<int>(RenderQueueFamily::Count); ++fam) {
                frameData[i].commandPools[fam] = VulkanCommandPool::create(static_cast<RenderQueueFamily>(fam));
            }
        }
    }

    void VulkanContext::createDescriptorPool() {
        DescriptorPool::Builder builder;
        builder.addDescriptorTypeCount(DescriptorType::ConstantBuffer, 1024);
        descriptorPool = builder.build();
    }

    void VulkanContext::recreateSwapChain() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device));

        if(swapchain != VK_NULL_HANDLE)
            cleanupSwapChain();

        createSwapChain();
        createFramebuffers();
    }


    void VulkanContext::cleanupSwapChain() {
        swapchainFramebuffers.clear();
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    template<typename T>
    static T VulkanContext::getExtensionFunction(VkInstance instance, const char *name) {
        auto func = (T) vkGetInstanceProcAddr(instance, name);
        BZ_ASSERT_CORE(func, "Unable to get {} function pointer!", name);
        return func;
    }

    VkPhysicalDevice VulkanContext::pickPhysicalDevice(const std::vector<const char*> &requiredDeviceExtensions) {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        uint32_t deviceCount = 0;
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

        std::vector<VkPhysicalDevice> devices(deviceCount);
        BZ_ASSERT_VK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
        for(const auto& device : devices) {
            queueFamilyIndices = findQueueFamilies(device);

            if(isPhysicalDeviceSuitable(device, requiredDeviceExtensions)) {
                physicalDevice = device;
                break;
            }
        }
        return physicalDevice;
    }

    bool VulkanContext::isPhysicalDeviceSuitable(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        bool hasRequiredExtensions = checkDeviceExtensionSupport(device, requiredExtensions);

        bool isSwapChainAdequate = false;
        if(hasRequiredExtensions) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty(); //TODO: have some requirements
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && queueFamilyIndices.isComplete() && hasRequiredExtensions && isSwapChainAdequate;
    }

    bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*> &requiredExtensions) const {
        uint32_t extensionCount;
        BZ_ASSERT_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        BZ_ASSERT_VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

        for(const char* extensionName : requiredExtensions) {
            bool extensionFound = false;
            for(const auto& extensionProp : availableExtensions) {
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

    RenderQueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const {
        RenderQueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto& queueFamily : queueFamilies) {
            if(queueFamily.queueCount > 0) {
                if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphicsFamily = i;
                if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    indices.computeFamily = i;
                if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    indices.transferFamily = i;
            }

            VkBool32 presentSupport = false;
            BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));
            if(queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }

            if(indices.isComplete()) break;
            i++;
        }
        return indices;
    }

    VulkanContext::SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) const {
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

    VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
        for(const auto& availableFormat : availableFormats) {
            if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
        for(const auto& availablePresentMode : availablePresentModes) {
            if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if(capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {
            int w, h;
            glfwGetFramebufferSize(windowHandle, &w, &h);
            VkExtent2D actualExtent = {w, h};

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    void VulkanContext::assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const {
        uint32_t layerCount;
        BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        
        std::vector<VkLayerProperties> availableLayers(layerCount);
        BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

        for(const char* layerName : requiredLayers) {
            bool layerFound = false;
            for(const auto& layerProperties : availableLayers) {
                if(strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound)
                BZ_ASSERT_ALWAYS_CORE("Requested Validation Layer '{}' but it was not found!", layerName);
        }
    }
}