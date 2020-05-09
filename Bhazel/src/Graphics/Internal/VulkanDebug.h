#pragma once

#include <vulkan/vulkan.h>


#ifdef BZ_GRAPHICS_DEBUG

namespace BZ {
    VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
}

#define BZ_ASSERT_VK(call) { \
    VkResult res; \
    (res = call); \
    if(res != VK_SUCCESS) \
        BZ_ASSERT_ALWAYS_CORE("VkResult is not Success! Error: {}.", static_cast<int>(res)); \
}

#define BZ_LOG_VK(call) { \
   VkResult res; \
    (res = call); \
    if(res != VK_SUCCESS) \
        BZ_LOG_CORE_ERROR("VkResult is not Success. Error: {}. File: {}. Line: {}.", static_cast<int>(res), __FILE__, __LINE__); \
}

#else
#define BZ_ASSERT_VK(call) call
#define BZ_LOG_VK(call) call
#endif