#pragma once

#include "Bhazel/Application.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"


namespace BZ {

    template<typename HandleT>
    class VulkanGpuObject {
    public:
        HandleT getNativeHandle() const {
            return nativeHandle;
        }

        VulkanContext& getGraphicsContext() const {
            return static_cast<VulkanContext &>(Application::getInstance().getGraphicsContext());
        }

    protected:
        HandleT nativeHandle;
    };
}