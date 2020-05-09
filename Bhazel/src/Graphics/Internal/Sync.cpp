#include "bzpch.h"

#include "Sync.h"
#include "Graphics/Internal/Device.h"


namespace BZ {

    void Fence::init(const Device &device, bool signaled) {
        this->device = &device;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        BZ_ASSERT_VK(vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &handle));
    }

    void Fence::destroy() {
        vkDestroyFence(device->getHandle(), handle, nullptr);
    }

    void Fence::waitFor(uint64 timeout) const {
        BZ_ASSERT_VK(vkWaitForFences(device->getHandle(), 1, &handle, VK_TRUE, timeout));
    }

    bool Fence::isSignaled() const {
        VkResult res = vkGetFenceStatus(device->getHandle(), handle);
        return res == VK_SUCCESS;
    }

    void Fence::reset() const {
        BZ_ASSERT_VK(vkResetFences(device->getHandle(), 1, &handle));
    }


    /*-------------------------------------------------------------------------------------------*/
    void Semaphore::init(const Device &device) {
        this->device = &device;
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        BZ_ASSERT_VK(vkCreateSemaphore(device.getHandle(), &semaphoreInfo, nullptr, &handle));
    }

    void Semaphore::destroy() {
        vkDestroySemaphore(device->getHandle(), handle, nullptr);
    }
}