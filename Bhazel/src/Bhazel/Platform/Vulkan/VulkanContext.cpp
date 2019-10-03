#include "bzpch.h"

#include "Bhazel/Platform/Vulkan/VulkanContext.h"
#include "Bhazel/Platform/Vulkan/VulkanRendererAPI.h"
#include "Bhazel/Platform/Vulkan/VulkanTexture.h"
#include "Bhazel/Platform/Vulkan/VulkanPipelineState.h"
#include "Bhazel/Platform/Vulkan/VulkanFramebuffer.h"
#include "Bhazel/Platform/Vulkan/VulkanBuffer.h"

#include "Bhazel/Renderer/RenderCommand.h"
#include "Bhazel/Renderer/Shader.h"
#include "Bhazel/Renderer/Buffer.h"

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

        vertexBuffer.reset();

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
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
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        physicalDevice = pickPhysicalDevice(requiredDeviceExtensions);
        BZ_ASSERT_CORE(physicalDevice != VK_NULL_HANDLE, "Couldn't find a suitable physical device!");

        createLogicalDevice(requiredDeviceExtensions);
        createSyncObjects();

        createSwapChain();
        createFramebuffers();
        initTestStuff();

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        BZ_LOG_CORE_INFO("Vulkan Context:");
        BZ_LOG_CORE_INFO("  Device Name: {}.", physicalDeviceProperties.deviceName);
        BZ_LOG_CORE_INFO("  Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion), VK_VERSION_MINOR(physicalDeviceProperties.apiVersion), VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
        BZ_LOG_CORE_INFO("  Driver Version: {}.{}.{}.", VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion), VK_VERSION_MINOR(physicalDeviceProperties.driverVersion), VK_VERSION_PATCH(physicalDeviceProperties.driverVersion));
        BZ_LOG_CORE_INFO("  VendorId: 0x{:04x}.", physicalDeviceProperties.vendorID);
        BZ_LOG_CORE_INFO("  DeviceId: 0x{:04x}.", physicalDeviceProperties.deviceID);

        rendererAPI = std::make_unique<VulkanRendererAPI>();
        RenderCommand::initRendererAPI(rendererAPI.get());
    }

    void VulkanContext::presentBuffer() {
        //glfwSwapBuffers(windowHandle);
        //TODO

        draw();
    }

    void VulkanContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);

        //TODO
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
        std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};
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
        BZ_ASSERT_VK(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain));

        //swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void VulkanContext::createFramebuffers() {
        //Get the images created for the swapchain
        std::vector<VkImage> swapChainImages;
        uint32_t imageCount;
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr));
        swapChainImages.resize(imageCount);
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()));

        //Create Views for the images
       swapChainFramebuffers.resize(imageCount);
       for(size_t i = 0; i < swapChainImages.size(); i++) {
            auto textureRef = VulkanTexture2D::wrap(swapChainImages[i], swapChainExtent.width, swapChainExtent.height);
            auto textureViewRef = TextureView::create(textureRef);
            swapChainFramebuffers[i] = Framebuffer::create({ textureViewRef });
        }
    }

    void VulkanContext::createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            BZ_ASSERT_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            BZ_ASSERT_VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            BZ_ASSERT_VK(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
        }
    }

    void VulkanContext::recreateSwapChain() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device));

        if(swapChain != VK_NULL_HANDLE)
            cleanupSwapChain();

        createSwapChain();
        createFramebuffers();

        //Render pass depends on the format of the swapchain (it's rare for the format to change, but still...)
        //Viewport and scissor rectangle size (possible to avoid if we set those to dynamic state)
        //Framebuffers and command buffers depend on swapchain images
        initTestStuff();
    }


    void VulkanContext::cleanupSwapChain() {
        pipelineState.reset();
        swapChainFramebuffers.clear();

        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroySwapchainKHR(device, swapChain, nullptr);
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

    VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto& queueFamily : queueFamilies) {
            if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            BZ_ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));
            if(queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }

            i++;

            if(indices.isComplete()) break;
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

    /////////////////////////////
    ///TODO : temporary stuff
    ////////////////////////////
    void VulkanContext::createCommandBuffers() {
        //Create command pool
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional
        BZ_ASSERT_VK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

        //Create command buffers
        commandBuffers.resize(swapChainFramebuffers.size());
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        BZ_ASSERT_VK(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));

        //Start command buffer recording
        for(size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            BZ_ASSERT_VK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

            const VulkanFramebuffer &vkFramebuffer = static_cast<const VulkanFramebuffer&>(*swapChainFramebuffers[i]);
            
            //Record a render pass
            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = vkFramebuffer.getNativeHandle().renderPassHandle;
            renderPassBeginInfo.framebuffer = vkFramebuffer.getNativeHandle().frameBufferHandle;
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = swapChainExtent;
            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearColor;
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            const VulkanPipelineState &vkPipeState = static_cast<const VulkanPipelineState &>(*pipelineState);
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeState.getNativeHandle());

            VkBuffer vkBuffers[] = { static_cast<const VulkanBuffer &>(*vertexBuffer).getNativeHandle() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vkBuffers, offsets);

            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            BZ_ASSERT_VK(vkEndCommandBuffer(commandBuffers[i]));
        }
    }

    void VulkanContext::initTestStuff() {
        PipelineStateData pipelineStateData;
        pipelineStateData.shader = Shader::createFromBlob("test", "shaders/bin/vert.spv", "shaders/bin/frag.spv");

        DataLayout layout = {
            {DataType::Float32, DataElements::Vec2, "POSITION"},
            {DataType::Float32, DataElements::Vec3, "COLOR"},
        };
        float data[] = {
            0.0f, -0.5f,
            1.0f, 0.0f, 0.0f,
            0.5f, 0.5f,
            0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f,
            0.0f, 0.0f, 1.0f,
        };

        vertexBuffer = Buffer::createVertexBuffer(data, sizeof(data), layout);

        pipelineStateData.dataLayout = layout;
        pipelineStateData.primitiveTopology = PrimitiveTopology::Triangles;
        pipelineStateData.viewports = { { 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height)} };
        pipelineStateData.blendingState.attachmentBlendingStates = { {} };
        pipelineStateData.framebuffer = swapChainFramebuffers[0]; //All the framebuffers have a similar VkRenderPass

        pipelineState = PipelineState::create(pipelineStateData);

        createCommandBuffers();
    }

    void VulkanContext::draw() {
        BZ_ASSERT_VK(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));

        //This index will be used to pick the correct command buffer
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to acquire image for presentation. Error: {}.", result);
            recreateSwapChain();
            return;
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        BZ_ASSERT_VK(vkResetFences(device, 1, &inFlightFences[currentFrame]));

        BZ_ASSERT_VK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to present image. Error: {}.", result);
            recreateSwapChain();
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}