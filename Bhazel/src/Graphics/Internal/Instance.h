#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Instance {
    public:
        Instance() = default;

        BZ_NON_COPYABLE(Instance);

        void init();
        void destroy();

        template<typename T>
        T getExtensionFunction(const char *name) {
            auto func = (T)vkGetInstanceProcAddr(handle, name);
            BZ_CRITICAL_ERROR_CORE(func, "Unable to get {} function pointer!", name);
            return func;
        }

        VkInstance getHandle() const { return handle; }

    private:
        VkInstance handle;

#ifdef BZ_GRAPHICS_DEBUG
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        static void checkValidationLayerSupport(const std::vector<const char*> &requiredLayers);
    };
}