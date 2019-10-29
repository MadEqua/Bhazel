#pragma once

#include "Core/Application.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace BZ {

    template<typename HandleT>
    class VulkanGpuObject {
    public:
        HandleT getNativeHandle() const {
            return nativeHandle;
        }

        static VulkanContext& getGraphicsContext() {
            return static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext());
        }

        static VkDevice getDevice() {
            return static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext()).getDevice().getNativeHandle();
        }

    protected:
        HandleT nativeHandle;
    };
}