#include "bzpch.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanConversions.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Buffer.h"

#include <GLFW/glfw3.h>


namespace BZ {

    VulkanContext::VulkanContext(void *windowHandle) :
        windowHandle(static_cast<GLFWwindow*>(windowHandle)) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null!");
    }

    VulkanContext::~VulkanContext() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device));

        cleanupSwapChain();
        cleanupFrameData();
        descriptorPool.reset();

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

        graphicsApi = std::make_unique<VulkanGraphicsApi>(*this);
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

        VkResult result = vkQueuePresentKHR(queueContainer.present.getNativeHandle(), &presentInfo);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to present image. Error: {}.", result);
            recreateSwapChain();
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frameData[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &swapchainCurrentImageIndex);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to acquire image for presentation. Error: {}.", result);
            recreateSwapChain();
        }

        //TODO: check correctness
        BZ_ASSERT_VK(vkWaitForFences(device, 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));
        for(const auto &pair : frameData[currentFrame].commandPoolsByFamily) {
            pair.second->reset();
        }
    }

    void VulkanContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);
        BZ_LOG_CORE_ERROR("Vulkan vsync not implemented!");
    }

    Ref<Framebuffer> VulkanContext::getCurrentFrameFramebuffer() {
        return swapchainFramebuffers[currentFrame];
    }

    VulkanCommandPool& VulkanContext::getCommandPool(QueueProperty property, uint32 frame, bool exclusive) {
        BZ_ASSERT_CORE(frame < MAX_FRAMES_IN_FLIGHT, "Invalid frame: {}!", frame);

        std::vector<const QueueFamily *> families;
        if(exclusive) {
            families = queueFamilyContainer.getFamiliesThatContainExclusively(property);
            if(families.empty()) {
                BZ_LOG_WARN("Requested a CommandPool for property {} and exclusive but there is none. Returning a non-exclusive one.", static_cast<int>(property));
                families = queueFamilyContainer.getFamiliesThatContain(property);
            }
        }
        else {
            families = queueFamilyContainer.getFamiliesThatContain(property);
        }
        return *frameData[frame].commandPoolsByFamily[families[0]->getIndex()];
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

    void VulkanContext::createLogicalDevice(const std::vector<const char *> &requiredDeviceExtensions) {
        auto processFamilies = [this](QueueProperty property, const QueueFamily **family, const QueueFamily **familyExclusive) {
            auto families = queueFamilyContainer.getFamiliesThatContain(property);
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
        BZ_ASSERT_VK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

        auto fillQueues = [this](const QueueFamily &family, const QueueFamily *familyExclusive, VulkanQueue &dstVulkanQueue, VulkanQueue &dstVulkanQueueExclusive) {
            VkQueue vkQueue;
            vkGetDeviceQueue(device, family.getIndex(), 0, &vkQueue);
            dstVulkanQueue = VulkanQueue(vkQueue, family);

            //If there is no exclusive queue, then fill with the same data as the non-exclusive queue.
            VkQueue vkQueueExclusive;
            vkGetDeviceQueue(device, familyExclusive ? familyExclusive->getIndex() : family.getIndex(), 0, &vkQueueExclusive);
            dstVulkanQueueExclusive = VulkanQueue(vkQueueExclusive, familyExclusive ? *familyExclusive : family);
        };

        fillQueues(*graphicsFamily, graphicsFamilyExclusive, queueContainer.graphics, queueContainerExclusive.graphics);
        fillQueues(*computeFamily, computeFamilyExclusive, queueContainer.compute, queueContainerExclusive.compute);
        fillQueues(*transferFamily, transferFamilyExclusive, queueContainer.transfer, queueContainerExclusive.transfer);
        fillQueues(*presentFamily, presentFamilyExclusive, queueContainer.present, queueContainerExclusive.present);
    }

    void VulkanContext::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
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

        if(queueContainer.graphics.getFamily().getIndex() != queueContainer.present.getFamily().getIndex()) {
            uint32_t queueFamilyIndicesArr[] = { queueContainer.graphics.getFamily().getIndex(), queueContainer.present.getFamily().getIndex() };
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
        }
        else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr; 
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

            for(const auto &fam : queueFamilyContainer) {
                if(fam.isInUse())
                    frameData[i].commandPoolsByFamily[fam.getIndex()] = VulkanCommandPool::create(fam);
            }
        }
    }

    void VulkanContext::createDescriptorPool() {
        VulkanDescriptorPool::Builder builder;
        builder.addDescriptorTypeCount(DescriptorType::ConstantBuffer, 1024);
        descriptorPool = builder.build();
    }

    void VulkanContext::recreateSwapChain() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device));

        if(swapchain != VK_NULL_HANDLE) {
            cleanupFramebuffers();
            cleanupSwapChain();
            cleanupFrameData();
        }

        createFrameData();
        createSwapChain();
        createFramebuffers();

        currentFrame = 0;
    }

    void VulkanContext::cleanupSwapChain() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void VulkanContext::cleanupFramebuffers() {
        swapchainFramebuffers.clear();
    }

    void VulkanContext::cleanupFrameData() {
        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(device, frameData[i].imageAvailableSemaphore, nullptr);
            vkDestroySemaphore(device, frameData[i].renderFinishedSemaphore, nullptr);
            vkDestroyFence(device, frameData[i].inFlightFence, nullptr);
            frameData[i].commandPoolsByFamily.clear();
        }
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
            queueFamilyContainer = getQueueFamilies(device);

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

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            queueFamilyContainer.hasAllProperties() && 
            hasRequiredExtensions && isSwapChainAdequate;
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

    QueueFamilyContainer VulkanContext::getQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyContainer queueFamilyContainer;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int idx = 0, propsIdx = 0;
        for(const auto& vkQueueFamilyProps : queueFamilies) {
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