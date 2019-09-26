#pragma once

#include <vulkan/vulkan.h>


#ifndef BZ_DIST

namespace BZ {
    VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
}

#define BZ_ASSERT_VK(call) { \
    VkResult res; \
    (res = call); \
    if(res != VK_SUCCESS) \
        BZ_ASSERT_ALWAYS_CORE("Vulkan Error. Code: 0x{0:04x}.", static_cast<int>(res)); \
}

#define BZ_LOG_VK(call) { \
   VkResult res; \
    (res = call); \
    if(res != VK_SUCCESS) \
        BZ_LOG_CORE_ERROR("Vulkan Error. Code: 0x{0:04x}.", static_cast<int>(res)); \
}

#else
#define BZ_ASSERT_VK(call) call
#define BZ_LOG_VK(call) call
#endif
