#include "bzpch.h"

#include "VulkanQueue.h"


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


    VulkanQueue::VulkanQueue(VkQueue queue, const QueueFamily &family) :
        family(family), queue(queue) {
        family.setInUse();
    }
}