#include "bzpch.h"

#include "VulkanSync.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"


namespace BZ {

    /*VulkanFence::VulkanFence(const VulkanDevice &device, bool signaled) {
        init(device, signaled);
    }

    VulkanFence::~VulkanFence() {
        destroy();
    }*/

    void VulkanFence::init(const VulkanDevice &device, bool signaled) {
        BZ_ASSERT_CORE(fence == VK_NULL_HANDLE, "Fence is already inited!");

        this->device = &device;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        BZ_ASSERT_VK(vkCreateFence(device.getNativeHandle(), &fenceInfo, nullptr, &fence));
    }

    void VulkanFence::destroy() {
        vkDestroyFence(device->getNativeHandle(), fence, nullptr);
        fence = VK_NULL_HANDLE;
    }

    void VulkanFence::waitFor(uint64 timeout) const {
        BZ_ASSERT_VK(vkWaitForFences(device->getNativeHandle(), 1, &fence, VK_TRUE, timeout));
    }

    bool VulkanFence::isSignaled() const {
        VkResult res = vkGetFenceStatus(device->getNativeHandle(), fence);
        return res == VK_SUCCESS;
    }

    void VulkanFence::reset() const {
        BZ_ASSERT_VK(vkResetFences(device->getNativeHandle(), 1, &fence));
    }


    /*VulkanSemaphore::VulkanSemaphore(const VulkanDevice &device) {
        init(device);
    }

    VulkanSemaphore::~VulkanSemaphore() {
        destroy();
    }*/

    void VulkanSemaphore::init(const VulkanDevice &device) {
        BZ_ASSERT_CORE(semaphore == VK_NULL_HANDLE, "Semaphore is already inited!");

        this->device = &device;
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        BZ_ASSERT_VK(vkCreateSemaphore(device.getNativeHandle(), &semaphoreInfo, nullptr, &semaphore));
    }

    void VulkanSemaphore::destroy() {
        vkDestroySemaphore(device->getNativeHandle(), semaphore, nullptr);
        semaphore = VK_NULL_HANDLE;
    }
}