#include "bzpch.h"

#include "Queue.h"
#include "Graphics/Internal/Device.h"


namespace BZ {

    QueueFamily::QueueFamily(uint32 index, uint32 queueCount, const std::vector<QueueProperty> &properties) :
        index(index), queueCount(queueCount) {
        for(const auto &prop : properties) {
            this->properties[static_cast<int>(prop)] = true;
        }
    }


    /*-------------------------------------------------------------------------------------------*/
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


    /*-------------------------------------------------------------------------------------------*/
    void Queue::init(const Device &device, const QueueFamily &family) {
        this->family = family;

        vkGetDeviceQueue(device.getHandle(), family.getIndex(), 0, &handle);
    }


    /*-------------------------------------------------------------------------------------------*/
    std::set<uint32> QueueContainer::getFamilyIndexesInUse() const {
        std::set<uint32> ret;
        for(uint32 i = 0; i < static_cast<uint32>(QueueProperty::Count); ++i) {
            ret.insert(queues[i].getFamily().getIndex());
        }
        return ret;
    }
}