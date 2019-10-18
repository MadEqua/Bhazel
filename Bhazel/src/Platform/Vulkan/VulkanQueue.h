#pragma once

#include "Platform/Vulkan/VulkanIncludes.h"
//#include "Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    enum class QueueProperty {
        Graphics,
        Compute,
        Transfer,
        Present,

        Count
    };

    //Queues of the same family are caracterized by having the same set of properties.
    class QueueFamily {
    public:
        QueueFamily() = default;
        QueueFamily(uint32 index, uint32 queueCount, const std::vector<QueueProperty> &properties);

        bool hasProperty(QueueProperty type) const { return properties[static_cast<int>(type)]; }
        uint32 getIndex() const { return index; }

        uint32 getPropertyCount() const { return static_cast<uint32>(properties.count()); }
        bool hasExclusiveProperty() const { return properties.count() == 1; }

        void setInUse() const { inUse = true; }
        bool isInUse() const { return inUse; }

    private:
        uint32 index;
        uint32 queueCount;
        std::bitset<static_cast<uint32>(QueueProperty::Count)> properties;
        mutable bool inUse = false;

        friend class QueueFamilyContainer;
    };


    //Contains the families present on a Device.
    class QueueFamilyContainer {
    public:
        QueueFamilyContainer() = default;
        void addFamily(const QueueFamily &family);

        std::vector<const QueueFamily*> getFamiliesThatContain(QueueProperty property) const;
        std::vector<const QueueFamily*> getFamiliesThatContainExclusively(QueueProperty property) const;

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


    class VulkanQueue {
    public:
        VulkanQueue() = default;
        VulkanQueue(VkQueue queue, const QueueFamily &family);

        const QueueFamily& getFamily() const { return family; }
        VkQueue getNativeHandle() const { return queue; }

    private:
        VkQueue queue;
        QueueFamily family;
    };

    //The handles may point to the same queues with no restrictions.
    struct QueueContainer {
        VulkanQueue graphics;
        VulkanQueue compute;
        VulkanQueue transfer;
        VulkanQueue presentImage;
    };
}