#include "bzpch.h"

#include "Graphics/Internal/Device.h"
#include "Queue.h"


namespace BZ {

QueueFamily::QueueFamily(uint32 index, uint32 queueCount, const std::vector<QueueProperty> &properties,
                         uint32 timestampValidBits) :
    index(index),
    queueCount(queueCount), timestampValidBits(timestampValidBits) {
    for (const auto &prop : properties) {
        this->properties[static_cast<int>(prop)] = true;
    }
}


/*-------------------------------------------------------------------------------------------*/
void QueueFamilyContainer::addFamily(const QueueFamily &family) {
    families.push_back(family);
    cummulativeProperties |= family.properties;
}

bool QueueFamilyContainer::hasAllProperties() const {
    return cummulativeProperties.count() == static_cast<int>(QueueProperty::Count);
}


/*-------------------------------------------------------------------------------------------*/
void Queue::init(const Device &device, const QueueFamily &family) {
    this->family = family;

    vkGetDeviceQueue(device.getHandle(), family.getIndex(), 0, &handle);
}

const Queue *QueueContainer::getQueueByFamilyIndex(uint32 familyIndex) const {
    for (uint32 i = 0; i < static_cast<uint32>(QueueProperty::Count); ++i) {
        if (queues[i].getFamily().getIndex() == familyIndex)
            return &queues[i];
    }
    BZ_ASSERT_ALWAYS_CORE("Queue family index {} is not being used by any Queue!", familyIndex);
    return nullptr;
}

/*-------------------------------------------------------------------------------------------*/
std::set<uint32> QueueContainer::getFamilyIndexesInUse() const {
    std::set<uint32> ret;
    for (uint32 i = 0; i < static_cast<uint32>(QueueProperty::Count); ++i) {
        ret.insert(queues[i].getFamily().getIndex());
    }
    return ret;
}
}