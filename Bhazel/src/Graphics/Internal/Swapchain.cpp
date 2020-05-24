#include "bzpch.h"

#include "Swapchain.h"

#include "Core/Application.h"

#include "Graphics/Internal/Device.h"
#include "Graphics/Internal/Surface.h"

#include "Graphics/Sync.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"


namespace BZ {

    void Swapchain::init(const Device &device, const Surface &surface) {
        this->device = &device;
        this->surface = &surface;
        internalInit();
    }

    void Swapchain::destroy() {
        framebuffers.clear();
        renderPass.reset();
        vkDestroySwapchainKHR(device->getHandle(), handle, nullptr);
        currentImageIndex = 0;
        aquired = false;
    }

    void Swapchain::recreate() {
        destroy();
        internalInit();
    }

    void Swapchain::aquireImage(const Ref<Semaphore> &imageAvailableSemaphore) {
        BZ_ASSERT_CORE(!aquired, "Aquiring image with one already aquired. Should present it first!");

        VkResult result = vkAcquireNextImageKHR(device->getHandle(), handle, 0, imageAvailableSemaphore->getHandle(), VK_NULL_HANDLE, &currentImageIndex);
        if(result < VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to acquire image for presentation. Error: {}.", result);
            recreate();
        }
        else if (result != VK_SUCCESS) {
            BZ_LOG_CORE_INFO("VulkanContext had no success acquiring image for presentation. Result was: {}.", result);
        }
        else {
            aquired = true;
        }
    }

    void Swapchain::presentImage(const Ref<Semaphore> &renderFinishedSemaphore) {
        BZ_ASSERT_CORE(aquired, "Presenting image with none aquired. Aquire one first!");

        VkSemaphore waitSemaphore[] = { renderFinishedSemaphore->getHandle() };
        VkSwapchainKHR swapchains[] = { handle };

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &currentImageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(device->getQueueContainer().present().getHandle(), &presentInfo);
        if(result != VK_SUCCESS) {
            BZ_LOG_CORE_ERROR("VulkanContext failed to present image. Error: {}.", result);
            recreate();
        }
        else {
            aquired = false;
        }
    }

    void Swapchain::internalInit() {
        const SwapChainSupportDetails &swapchainSupport = device->getPhysicalDevice().getSwapChainSupportDetails();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);

        imageFormat = surfaceFormat.format;
        //if(imageFormat == VK_FORMAT_R8G8B8A8_SRGB)
        //    imageFormatLinear = VK_FORMAT_R8G8B8A8_UNORM;
        //else if(imageFormat == VK_FORMAT_B8G8R8A8_SRGB)
        //    imageFormatLinear = VK_FORMAT_B8G8R8A8_UNORM;
        //else
        //    BZ_ASSERT_ALWAYS_CORE("Not implemented!");

        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
        extent = chooseSwapExtent(swapchainSupport.surfaceCapabilities);

        //Try to go for 3 images minimum.
        uint32_t imageCount = std::max(swapchainSupport.surfaceCapabilities.minImageCount, 3u);
        if(swapchainSupport.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.surfaceCapabilities.maxImageCount) {
            imageCount = swapchainSupport.surfaceCapabilities.maxImageCount;
        }

        BZ_LOG_CORE_INFO("Swapchain needs a minImageCount of {} and a maxImageCount of {}. Picked an image count of {}.", 
            swapchainSupport.surfaceCapabilities.minImageCount, swapchainSupport.surfaceCapabilities.maxImageCount, imageCount);

        //VkFormat possibleFormats[] = { imageFormat, imageFormatLinear };
        //VkImageFormatListCreateInfoKHR imageFormatListCreateInfo = {};
        //imageFormatListCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR;
        //imageFormatListCreateInfo.viewFormatCount = 2;
        //imageFormatListCreateInfo.pViewFormats = possibleFormats;

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = surface->getHandle();
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        //wapChainCreateInfo.flags = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;
        //swapChainCreateInfo.pNext = &imageFormatListCreateInfo;

        const QueueContainer &queueContainer = device->getQueueContainer();
        if(queueContainer.graphics().getFamily().getIndex() != queueContainer.present().getFamily().getIndex()) {
            uint32_t queueFamilyIndicesArr[] = { queueContainer.graphics().getFamily().getIndex(), queueContainer.present().getFamily().getIndex() };
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
        }
        else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;
        }
        swapChainCreateInfo.preTransform = swapchainSupport.surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        BZ_ASSERT_VK(vkCreateSwapchainKHR(device->getHandle(), &swapChainCreateInfo, nullptr, &handle));

        createFramebuffers();
    }

    void Swapchain::createFramebuffers() {
        //Get the images created for the swapchain
        std::vector<VkImage> swapChainImages;
        uint32_t imageCount;
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device->getHandle(), handle, &imageCount, nullptr));
        swapChainImages.resize(imageCount);
        BZ_ASSERT_VK(vkGetSwapchainImagesKHR(device->getHandle(), handle, &imageCount, swapChainImages.data()));

        //Create the Swapchain default RenderPass.
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = imageFormat;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.loadOperatorColorAndDepth = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDesc.storeOperatorColorAndDepth = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDesc.loadOperatorStencil = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.storeOperatorStencil = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachmentDesc.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

        renderPass = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });

        //colorAttachmentDesc.format = imageFormatLinear;
        //renderPassLinear = RenderPass::create({ colorAttachmentDesc }, { subPassDesc });

        //Create Views and Framebuffers for the images.
        framebuffers.resize(imageCount);
        //framebuffersLinear.resize(imageCount);
        for(size_t i = 0; i < swapChainImages.size(); i++) {
            auto textureRef = Texture2D::wrap(swapChainImages[i], extent.width, extent.height, imageFormat);
            
            auto textureViewRef = TextureView::create(textureRef);
            framebuffers[i] = Framebuffer::create(renderPass, { textureViewRef }, glm::uvec3(extent.width, extent.height, 1));

            //Force view to linear format.
            //auto textureViewLinearRef = TextureView::create(textureRef, imageFormatLinear);
            //framebuffersLinear[i] = Framebuffer::create(renderPassLinear, { textureViewLinearRef }, glm::uvec3(extent.width, extent.height, 1));
        }
    }

    VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for(const auto &availableFormat : availableFormats) {
            if((availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB || availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB) && 
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                BZ_LOG_CORE_INFO("Found SRGB Surface format for Swapchain.");
                return availableFormat;
            }
        }
        BZ_LOG_CORE_WARN("Could not find SRGB Surface format for Swapchain.");
        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for(const auto &availablePresentMode : availablePresentModes) {
            if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                BZ_LOG_CORE_INFO("Found Mailbox present mode for Swapchain.");
                return availablePresentMode;
            }
        }
        BZ_LOG_CORE_WARN("Could not find Mailbox present mode for Swapchain. Using FIFO.");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
        if(surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            return surfaceCapabilities.currentExtent;
        }
        else {
            //This case means that surface size will be determined by the extent of the swapchain.
            //Should never happen because we already created a surface with the size of the window.
            BZ_ASSERT_ALWAYS_CORE("Not implemented!");
            return {0, 0};
        }
    }
}