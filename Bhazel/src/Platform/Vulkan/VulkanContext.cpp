#include "bzpch.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include <GLFW/glfw3.h>


namespace BZ {

    VulkanContext::VulkanContext(void *windowHandle) :
        windowHandle(static_cast<GLFWwindow*>(windowHandle)) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null!");
    }

    VulkanContext::~VulkanContext() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));

        cleanupFrameData();
        descriptorPool.reset();

        vkDestroyDevice(device.getNativeHandle(), nullptr);
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
        createDevice();
        createFrameData();
        createSwapchain();
        createDescriptorPool();

        graphicsApi = std::make_unique<VulkanGraphicsApi>(*this);
    }

    void VulkanContext::presentBuffer() {
        swapchain.presentImage(frameData[currentFrame].renderFinishedSemaphore);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        swapchain.aquireImage(frameData[currentFrame].imageAvailableSemaphore);

        //TODO: check correctness
        BZ_ASSERT_VK(vkWaitForFences(device.getNativeHandle(), 1, &frameData[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX));
        for(const auto &pair : frameData[currentFrame].commandPoolsByFamily) {
            pair.second->reset();
        }
    }

    void VulkanContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);
        BZ_LOG_CORE_ERROR("Vulkan vsync not implemented!");
    }

    Ref<Framebuffer> VulkanContext::getCurrentFrameFramebuffer() {
        return swapchain.getFramebuffer(currentFrame);
    }

    Ref<Framebuffer> VulkanContext::getFramebuffer(uint32 frameIdx) {
        return swapchain.getFramebuffer(frameIdx);
    }

    VulkanCommandPool& VulkanContext::getCommandPool(QueueProperty property, uint32 frame, bool exclusive) {
        BZ_ASSERT_CORE(frame < MAX_FRAMES_IN_FLIGHT, "Invalid frame: {}!", frame);

        std::vector<const QueueFamily *> families;
        if(exclusive) {
            families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContainExclusively(property);
            if(families.empty()) {
                BZ_LOG_WARN("Requested a CommandPool for property {} and exclusive but there is none. Returning a non-exclusive one.", static_cast<int>(property));
                families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContain(property);
            }
        }
        else {
            families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContain(property);
        }
        return *frameData[frame].commandPoolsByFamily[families[0]->getIndex()];
    }

    uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        return physicalDevice.findMemoryType(typeFilter, properties);
    }

    void VulkanContext::onWindowResize(WindowResizedEvent& e) {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));

        swapchain.recreate();
        cleanupFrameData();
        createFrameData();

        currentFrame = 0;
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

    void VulkanContext::createDevice() {
        const std::vector<const char *> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        physicalDevice.init(instance, surface, requiredDeviceExtensions);
        device.init(physicalDevice, surface, requiredDeviceExtensions); 
    }

    void VulkanContext::createSwapchain() {
        swapchain.init(device, surface);
        swapchain.aquireImage(frameData[currentFrame].imageAvailableSemaphore);
    }

    void VulkanContext::createFrameData() {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            BZ_ASSERT_VK(vkCreateSemaphore(device.getNativeHandle(), &semaphoreInfo, nullptr, &frameData[i].imageAvailableSemaphore));
            BZ_ASSERT_VK(vkCreateSemaphore(device.getNativeHandle(), &semaphoreInfo, nullptr, &frameData[i].renderFinishedSemaphore));
            BZ_ASSERT_VK(vkCreateFence(device.getNativeHandle(), &fenceInfo, nullptr, &frameData[i].inFlightFence));

            for(const auto &fam : physicalDevice.getQueueFamilyContainer()) {
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

    void VulkanContext::cleanupFrameData() {
        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(device.getNativeHandle(), frameData[i].imageAvailableSemaphore, nullptr);
            vkDestroySemaphore(device.getNativeHandle(), frameData[i].renderFinishedSemaphore, nullptr);
            vkDestroyFence(device.getNativeHandle(), frameData[i].inFlightFence, nullptr);
            frameData[i].commandPoolsByFamily.clear();
        }
    }

    template<typename T>
    static T VulkanContext::getExtensionFunction(VkInstance instance, const char *name) {
        auto func = (T) vkGetInstanceProcAddr(instance, name);
        BZ_ASSERT_CORE(func, "Unable to get {} function pointer!", name);
        return func;
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