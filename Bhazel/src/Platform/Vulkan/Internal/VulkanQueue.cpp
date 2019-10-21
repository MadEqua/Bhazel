#include "bzpch.h"

#include "VulkanQueue.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"


namespace BZ {

    QueueFamily::QueueFamily(uint32 index, uint32 queueCount, const std::vector<QueueProperty> &properties) :
        index(index), queueCount(queueCount) {
        for(const auto &prop : properties) {
            this->properties[static_cast<int>(prop)] = true;
        }
    }


    void QueueFamilyContainer::addFamily(const QueueFamily &family) {
        families.push_back(family);
        cummulativeProperties |= family.properties;
    }

    std::vector<const QueueFamily *> QueueFamilyContainer::getFamiliesThatContain(QueueProperty property) const {
        std::vector<const QueueFamily *> result;
        for(const auto &fam : families)
            if(fam.hasProperty(property))
                result.push_back(&fam);
        return result;
    }

    std::vector<const QueueFamily *> QueueFamilyContainer::getFamiliesThatContainExclusively(QueueProperty property) const {
        std::vector<const QueueFamily *> result;
        for(const auto &fam : families)
            if(fam.hasProperty(property) && fam.hasExclusiveProperty())
                result.push_back(&fam);
        return result;
    }

    bool QueueFamilyContainer::hasAllProperties() const {
        return cummulativeProperties.count() == static_cast<int>(QueueProperty::Count);
    }


    /*VulkanQueue::VulkanQueue(const VulkanDevice &device, const QueueFamily &family) {
        init(device, family);
    }*/

    void VulkanQueue::init(const VulkanDevice &device, const QueueFamily &family) {
        BZ_ASSERT_CORE(queue == VK_NULL_HANDLE, "Queue is already inited!");

        this->family = family;
        family.setInUse();

        vkGetDeviceQueue(device.getNativeHandle(), family.getIndex(), 0, &queue);
    }
}