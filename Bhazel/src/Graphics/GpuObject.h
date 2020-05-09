#pragma once

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


namespace BZ {

    template<typename HandleT>
    struct GpuObject {

        GpuObject() = default;
        ~GpuObject() = default;

        BZ_NON_COPYABLE(GpuObject);

        static GraphicsContext& getGraphicsContext() {
            return Application::get().getGraphicsContext();
        }

        static VkDevice getVkDevice() {
            return Application::get().getGraphicsContext().getDevice().getHandle();
        }

        HandleT getHandle() const {
            return handle;
        }

    protected:
        HandleT handle;
    };
}