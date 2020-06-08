#pragma once

#include <bzpch.h>

#include "VulkanDebug.h"


#ifdef BZ_GRAPHICS_DEBUG

namespace BZ {
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {

    const char *typeString;
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            typeString = "General";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            typeString = "Validation";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            typeString = "Performance";
            break;
        default:
            typeString = "Unknown";
    }

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            BZ_LOG_CORE_ERROR("Vulkan Debug - Id: {}. Id Name: {}. Type: {}. Severity: Error. Message: {}",
                              pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, typeString,
                              pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            BZ_LOG_CORE_WARN("Vulkan Debug - Id: {}. Id Name: {}. Type: {}. Severity: Warning. Message: {}",
                             pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, typeString,
                             pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            BZ_LOG_CORE_INFO("Vulkan Debug - Id: {}. Id Name: {}. Type: {}. Severity: Info. Message: {}",
                             pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, typeString,
                             pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            BZ_LOG_CORE_INFO("Vulkan Debug - Id: {}. Id Name: {}. Type: {}. Severity: Verbose. Message: {}",
                             pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, typeString,
                             pCallbackData->pMessage);
            break;
        default:
            BZ_LOG_CORE_INFO("Vulkan Debug - Id: {}. Id Name: {}. Type: {}. Severity: Unknown. Message: {}",
                             pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, typeString,
                             pCallbackData->pMessage);
    }
    return VK_FALSE;
}
}

#endif