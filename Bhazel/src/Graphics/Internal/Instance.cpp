#include "bzpch.h"

#include "Instance.h"

#include <GLFW/glfw3.h>


namespace BZ {

void Instance::init() {
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

    // Ask GLFW which extensions it needs for platform specific stuff.
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> requiredInstanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef BZ_GRAPHICS_DEBUG
    // Request the debug utils extension. This way we can handle validation layers messages with Bhazel loggers.
    requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Request a validation layer
    const std::vector<const char *> layersToRequest = { "VK_LAYER_KHRONOS_validation" };
    checkValidationLayerSupport(layersToRequest);

    createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
    createInfo.ppEnabledLayerNames = layersToRequest.data();
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

    BZ_ASSERT_VK(vkCreateInstance(&createInfo, nullptr, &handle));

#ifdef BZ_GRAPHICS_DEBUG
    // Set the debug callback function
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
    debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugUtilsCreateInfo.pfnUserCallback = vulkanCallbackFunction;
    debugUtilsCreateInfo.pUserData = nullptr;

    auto func = getExtensionFunction<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
    BZ_ASSERT_VK(func(handle, &debugUtilsCreateInfo, nullptr, &debugMessenger));
#endif
}

void Instance::destroy() {
#ifdef BZ_GRAPHICS_DEBUG
    auto func = getExtensionFunction<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
    func(handle, debugMessenger, nullptr);
#endif
    vkDestroyInstance(handle, nullptr);
}

void Instance::checkValidationLayerSupport(const std::vector<const char *> &requiredLayers) {
    uint32_t layerCount;
    BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

    std::vector<VkLayerProperties> availableLayers(layerCount);
    BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    for (const char *layerName : requiredLayers) {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        BZ_CRITICAL_ERROR_CORE(layerFound, "Requested Validation Layer '{}' but it was not found!", layerName);
    }
}
}