#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Device;
   
    struct DescriptorPoolInitData {
        VkDescriptorType type;
        uint32 count;
    };

    //Internal only, not exposed to upper layers.
    class DescriptorPool {
    public:
        DescriptorPool() = default;

        BZ_NON_COPYABLE(DescriptorPool);

        void init(const Device &device, const std::initializer_list<DescriptorPoolInitData> &initDatas, uint32 maxSets);
        void destroy();

        void reset();

        VkDescriptorPool getHandle() const { return handle; }

    private:
        const Device *device;
        VkDescriptorPool handle;
    };
}
