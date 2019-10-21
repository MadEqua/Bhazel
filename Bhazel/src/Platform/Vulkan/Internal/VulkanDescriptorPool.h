#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    class VulkanDevice;

    //Internal only, not exposed to upper layers.
    class VulkanDescriptorPool {
    public:
        class Builder {
        public:
            Builder& addDescriptorTypeCount(DescriptorType type, uint32 count);
            //Ref<VulkanDescriptorPool> build() const;

        private:
            uint32 countPerType[static_cast<int>(DescriptorType::Count)] = { 0 };
            uint32 totalCount = 0;

            friend class VulkanDescriptorPool;
        };

        void init(const VulkanDevice &device, const Builder &builder);
        void destroy();

        void reset();

        VkDescriptorPool getNativeHandle() const { return descriptorPool; }

    private:
        VkDevice device;
        VkDescriptorPool descriptorPool;
    };
}
