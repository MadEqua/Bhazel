#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

class Device;

enum class QueueProperty {
    Graphics,
    Compute,
    Transfer,
    Present,

    Count
};

// Queues of the same family are caracterized by having the same set of properties.
class QueueFamily {
  public:
    QueueFamily() = default;
    QueueFamily(uint32 index, uint32 queueCount, const std::vector<QueueProperty> &properties,
                uint32 timestampValidBits);

    bool hasProperty(QueueProperty type) const { return properties[static_cast<int>(type)]; }
    uint32 getIndex() const { return index; }

    uint32 getPropertyCount() const { return static_cast<uint32>(properties.count()); }
    bool hasExclusiveProperty() const { return properties.count() == 1; }

    uint32 getTimestampValidBits() const { return timestampValidBits; }
    bool canUseTimestamps() const { return timestampValidBits != 0; }

  private:
    uint32 index;
    uint32 queueCount;
    uint32 timestampValidBits;
    std::bitset<static_cast<uint32>(QueueProperty::Count)> properties;

    friend class QueueFamilyContainer;
};


/*-------------------------------------------------------------------------------------------*/
// Contains the families present on a Device.
class QueueFamilyContainer {
  public:
    QueueFamilyContainer() = default;
    void addFamily(const QueueFamily &family);

    bool hasAllProperties() const;

    uint32 getCount() const { return static_cast<uint32>(families.size()); }

    std::vector<QueueFamily>::iterator begin() { return families.begin(); }
    std::vector<QueueFamily>::iterator end() { return families.end(); }
    std::vector<QueueFamily>::const_iterator begin() const { return families.cbegin(); }
    std::vector<QueueFamily>::const_iterator end() const { return families.cend(); }

  private:
    std::vector<QueueFamily> families;
    std::bitset<static_cast<uint32>(QueueProperty::Count)> cummulativeProperties;
};


/*-------------------------------------------------------------------------------------------*/
class Queue {
  public:
    Queue() = default;

    BZ_NON_COPYABLE(Queue);

    void init(const Device &device, const QueueFamily &family);

    const QueueFamily &getFamily() const { return family; }
    VkQueue getHandle() const { return handle; }

  private:
    VkQueue handle;
    QueueFamily family;
};


/*-------------------------------------------------------------------------------------------*/
// The handles may point to the same queues with no restrictions.
class QueueContainer {
  public:
    const Queue &graphics() const { return queues[static_cast<int>(QueueProperty::Graphics)]; }
    Queue &graphics() { return queues[static_cast<int>(QueueProperty::Graphics)]; }

    const Queue &compute() const { return queues[static_cast<int>(QueueProperty::Compute)]; }
    Queue &compute() { return queues[static_cast<int>(QueueProperty::Compute)]; }

    const Queue &transfer() const { return queues[static_cast<int>(QueueProperty::Transfer)]; }
    Queue &transfer() { return queues[static_cast<int>(QueueProperty::Transfer)]; }

    const Queue &present() const { return queues[static_cast<int>(QueueProperty::Present)]; }
    Queue &present() { return queues[static_cast<int>(QueueProperty::Present)]; }

    const Queue &getQueueByProperty(QueueProperty property) const { return queues[static_cast<int>(property)]; }
    const Queue *getQueueByFamilyIndex(uint32 familyIndex) const;

    std::set<uint32> getFamilyIndexesInUse() const;

  private:
    Queue queues[static_cast<int>(QueueProperty::Count)];
};
}