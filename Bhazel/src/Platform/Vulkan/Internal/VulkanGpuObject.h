#pragma once

#include "Core/Application.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace BZ {

    template<typename HandleT>
    struct VulkanGpuObject {
        HandleT getNativeHandle() const {
            return nativeHandle;
        }

        static VulkanContext& getGraphicsContext() {
            return static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext());
        }

        static VkDevice getDevice() {
            return static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext()).getDevice().getNativeHandle();
        }

        HandleT nativeHandle;
    };
}