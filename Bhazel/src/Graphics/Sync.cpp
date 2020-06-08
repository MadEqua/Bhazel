#include "bzpch.h"

#include "Sync.h"

#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"


namespace BZ {

Ref<Fence> Fence::create(bool signaled) {
    return MakeRef<Fence>(signaled);
}

Fence::Fence(bool signaled) {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    BZ_ASSERT_VK(vkCreateFence(BZ_GRAPHICS_DEVICE.getHandle(), &fenceInfo, nullptr, &handle));
}

Fence::~Fence() {
    vkDestroyFence(BZ_GRAPHICS_DEVICE.getHandle(), handle, nullptr);
}

void Fence::waitFor(uint64 timeout) const {
    BZ_ASSERT_VK(vkWaitForFences(BZ_GRAPHICS_DEVICE.getHandle(), 1, &handle, VK_TRUE, timeout));
}

bool Fence::isSignaled() const {
    VkResult res = vkGetFenceStatus(BZ_GRAPHICS_DEVICE.getHandle(), handle);
    return res == VK_SUCCESS;
}

void Fence::reset() const {
    BZ_ASSERT_VK(vkResetFences(BZ_GRAPHICS_DEVICE.getHandle(), 1, &handle));
}


/*-------------------------------------------------------------------------------------------*/
Ref<Semaphore> Semaphore::create() {
    return MakeRef<Semaphore>();
}

Semaphore::Semaphore() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    BZ_ASSERT_VK(vkCreateSemaphore(BZ_GRAPHICS_DEVICE.getHandle(), &semaphoreInfo, nullptr, &handle));
}

Semaphore::~Semaphore() {
    vkDestroySemaphore(BZ_GRAPHICS_DEVICE.getHandle(), handle, nullptr);
}
}