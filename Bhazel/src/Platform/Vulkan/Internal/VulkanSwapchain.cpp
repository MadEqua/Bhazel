#include "bzpch.h"

#include "VulkanSwapchain.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/Internal/VulkanSurface.h"
#include "Platform/Vulkan/Internal/VulkanSync.h"


namespace BZ {

    /*VulkanSwapchain::VulkanSwapchain(const VulkanDevice &device, const VulkanSurface &surface) {
        init(device, surface);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        destroy();
    }*/

    void VulkanSwapchain::init(const VulkanDevice &device, const VulkanSurface &surface) {
        this->device = &device;
        this->surface = &surface;
        internalInit();
    }

    void VulkanSwapchain::destroy() {
        framebuffers.clear();
        vkDestroySwapchainKHR(device->getNativeHandle(), swapchain, nullptr);
        currentImageIndex = 0;
        swapchain = VK_NULL_HANDLE;
    }

    void VulkanSwapchain::recreate() {
        destroy();
        internalInit();
    }

    void VulkanSwapchain::aquireImage(const VulkanSemaphore &imageAvailableSemaphore) {
        VkResult result = vkAcquireNextImageKHR(device->getNativeHandle(), swapchain, UINT64_MAX, imageAvailableSemaphore.getNativeHandle(), VK_NULL_HANDLE, &currentImageIndex);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to acquire image for presentation. Error: {}.", result);
            recreate();
        }
    }

    void VulkanSwapchain::presentImage(const VulkanSemaphore &renderFinishedSemaphore) {
        VkSemaphore waitSemaphore[] = { renderFinishedSemaphore.getNativeHandle() };
        VkSwapchainKHR swapchains[] = { swapchain };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &currentImageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(device->getQueueContainer().present.getNativeHandle(), &presentInfo);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to present image. Error: {}.", result);
            recreate();
        }
    }

    void VulkanSwapchain::internalInit() {
        const SwapChainSupportDetails &swapchainSupport = device->getPhysicalDevice().getSwapChainSupportDetails();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
        extent = chooseSwapExtent(swapchainSupport.capabilities);
        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if(swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = surface->getNativeHandle();
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueContainer &queueContainer = device->getQueueContainer();
        if(queueContainer.graphics.getFamily().getIndex() != queueContainer.present.getFamily().getIndex()) {
            uint32_t queueFamilyIndicesArr[] = { queueContainer.graphics.getFamily().getIndex(), queueContainer.present.getFamily().getIndex() };
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
        }
        else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;
        }
        swapChainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        BZ_ASSERT_VK(vkCreateSwapchainKHR(device->getNativeHandle(), &swapChainCreateInfo, nullptr, &swapchain));

        imageFormat = surfaceFormat.format;

        createFramebuffers();
    }

    void VulkanSwapchain::createFramebuffers() {
        //Get the images created for the swapchain
        std::vector<VkImage> swapChainImages;
        uint32_t imageCount;
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device->getNativeHandle(), swapchain, &imageCount, nullptr));
        swapChainImages.resize(imageCount);
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device->getNativeHandle(), swapchain, &imageCount, swapChainImages.data()));

        //Create Views for the images
        framebuffers.resize(imageCount);
        for(size_t i = 0; i < swapChainImages.size(); i++) {
            auto textureRef = VulkanTexture2D::wrap(swapChainImages[i], extent.width, extent.height, imageFormat);
            auto textureViewRef = TextureView::create(textureRef);

            AttachmentDescription attachmentDesc;
            attachmentDesc.format = textureRef->getFormat();
            attachmentDesc.samples = 1;
            attachmentDesc.loadOperatorColorAndDepth = LoadOperation::Clear;
            attachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
            attachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
            attachmentDesc.storeOperatorStencil = StoreOperation::DontCare;
            attachmentDesc.initialLayout = TextureLayout::Undefined;
            attachmentDesc.finalLayout = TextureLayout::Present;

            Framebuffer::Builder builder;
            builder.addColorAttachment(attachmentDesc, textureViewRef);
            builder.setDimensions(glm::ivec3(extent.width, extent.height, 1));

            framebuffers[i] = builder.build();
        }
    }

    VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for(const auto &availableFormat : availableFormats) {
            if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for(const auto &availablePresentMode : availablePresentModes) {
            if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if(capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {
            //TODO
            /*int w, h;
            glfwGetFramebufferSize(windowHandle, &w, &h);
            VkExtent2D actualExtent = { w, h };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;*/
        }
    }
}