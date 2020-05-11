#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/DescriptorSet.h"


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

        DescriptorSet& getDescriptorSet(const Ref<DescriptorSetLayout> &layout);

        void reset();

        VkDescriptorPool getHandle() const { return handle; }

    private:
        const Device *device;
        VkDescriptorPool handle;

        uint32 maxSets;

        DescriptorSet *sets;
        uint32 nextFreeIndex;
    };
}
