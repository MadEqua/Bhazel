#include "bzpch.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Graphics/Color.h"

#include <GLFW/glfw3.h>


namespace BZ {

    VulkanContext::VulkanContext(void *windowHandle) :
        windowHandle(static_cast<GLFWwindow*>(windowHandle)) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null!");
    }

    VulkanContext::~VulkanContext() {
        colorTextureView.reset();
        depthTextureView.reset();
        mainFramebuffer.reset();
        mainRenderPass.reset();
        descriptorPool.destroy();
        cleanupFrameData();
        swapchain.destroy();
        surface.destroy();
        vmaDestroyAllocator(memoryAllocator);
        device.destroy();

#ifndef BZ_DIST
        auto func = getExtensionFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(instance, "vkDestroyDebugUtilsMessengerEXT");
        func(instance, debugMessenger, nullptr);
#endif

        vkDestroyInstance(instance, nullptr);
    }

    void VulkanContext::init() {
        createInstance();

        surface.init(instance, *windowHandle);

        const std::vector<const char *> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        physicalDevice.init(instance, surface, requiredDeviceExtensions);
        device.init(physicalDevice, requiredDeviceExtensions);

        //Init VulkanMemoryAllocator lib.
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice.getNativeHandle();
        allocatorInfo.device = device.getNativeHandle();
        vmaCreateAllocator(&allocatorInfo, &memoryAllocator);

        createFrameData();

        VulkanDescriptorPool::Builder builder;
        builder.addDescriptorTypeCount(DescriptorType::ConstantBuffer, 512);
        builder.addDescriptorTypeCount(DescriptorType::CombinedTextureSampler, 512);
        builder.addDescriptorTypeCount(DescriptorType::Sampler, 512);
        builder.addDescriptorTypeCount(DescriptorType::SampledTexture, 512);
        descriptorPool.init(device, builder);

        //Create the RenderPass
        AttachmentDescription colorAttachmentDesc;
        colorAttachmentDesc.format = TextureFormatEnum::R32G32B32A32_SFLOAT;
        colorAttachmentDesc.samples = 1;
        colorAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::Load;
        colorAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        colorAttachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
        colorAttachmentDesc.storeOperatorStencil = StoreOperation::DontCare;
        colorAttachmentDesc.initialLayout = TextureLayout::Undefined;
        colorAttachmentDesc.finalLayout = TextureLayout::ShaderReadOnlyOptimal;
        colorAttachmentDesc.clearValues.floating = { 0.0f, 0.0f, 0.0f, 1.0f };

        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = TextureFormatEnum::D24S8;
        depthStencilAttachmentDesc.samples = 1;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::Load;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        depthStencilAttachmentDesc.loadOperatorStencil = LoadOperation::Load;
        depthStencilAttachmentDesc.storeOperatorStencil = StoreOperation::Store;
        depthStencilAttachmentDesc.initialLayout = TextureLayout::Undefined;
        depthStencilAttachmentDesc.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
        depthStencilAttachmentDesc.clearValues.floating.x = 1.0f;
        depthStencilAttachmentDesc.clearValues.integer.y = 0;

        SubPassDescription subPassDesc;
        subPassDesc.colorAttachmentsRefs = { { 0, TextureLayout::ColorAttachmentOptimal } };
        subPassDesc.depthStencilAttachmentsRef = { 1, TextureLayout::DepthStencilAttachmentOptimal };

        mainRenderPass = RenderPass::create({ colorAttachmentDesc, depthStencilAttachmentDesc }, { subPassDesc });

        //TODO: get the dimensions from a better place
        glm::ivec2 dimensions;
        glfwGetWindowSize(windowHandle, &dimensions.x, &dimensions.y);
        auto colorTexture = Texture2D::createRenderTarget(dimensions.x, dimensions.y, 1, colorAttachmentDesc.format);
        colorTextureView = TextureView::create(colorTexture);
        auto depthTexture = Texture2D::createRenderTarget(dimensions.x, dimensions.y, 1, depthStencilAttachmentDesc.format);
        depthTextureView = TextureView::create(depthTexture);

        mainFramebuffer = Framebuffer::create(mainRenderPass, { colorTextureView, depthTextureView }, glm::ivec3(dimensions.x, dimensions.y, 1));

        swapchain.init(device, surface);
    }

    void VulkanContext::setVSync(bool enabled) {
        GraphicsContext::setVSync(enabled);
        BZ_LOG_CORE_ERROR("Vulkan vsync not implemented!");
    }

    VulkanCommandPool& VulkanContext::getCurrentFrameCommandPool(QueueProperty property, bool exclusive) {
        std::vector<const QueueFamily *> families;
        if(exclusive) {
            families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContainExclusively(property);
            if(families.empty()) {
                BZ_LOG_WARN("Requested a CommandPool for property {} and exclusive but there is none. Returning a non-exclusive one.", static_cast<int>(property));
                families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContain(property);
            }
        }
        else {
            families = physicalDevice.getQueueFamilyContainer().getFamiliesThatContain(property);
        }
        return frameDatas[currentFrameIndex].commandPoolsByFamily[families[0]->getIndex()];
    }

    void VulkanContext::onWindowResize(const WindowResizedEvent& e) {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));

        swapchain.recreate();
        cleanupFrameData();
        createFrameData();

        currentFrameIndex = 0;
    }

    void VulkanContext::createInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = nullptr;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Bhazel Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //Ask GLFW which extensions it needs for platform specific stuff.
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredInstanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef BZ_DIST

        //Request the debug utils extension. This way we can handle validation layers messages with Bhazel loggers.
        requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //Request a validation layer
        const std::vector<const char*> layersToRequest = {
            "VK_LAYER_KHRONOS_validation"
        };
        assertValidationLayerSupport(layersToRequest);

        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
        createInfo.ppEnabledLayerNames = layersToRequest.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

        BZ_ASSERT_VK(vkCreateInstance(&createInfo, nullptr, &instance));

#ifndef BZ_DIST
        //Set the debug callback function
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = vulkanCallbackFunction;
        debugUtilsCreateInfo.pUserData = nullptr;

        auto func = getExtensionFunction<PFN_vkCreateDebugUtilsMessengerEXT>(instance, "vkCreateDebugUtilsMessengerEXT");
        BZ_ASSERT_VK(func(instance, &debugUtilsCreateInfo, nullptr, &debugMessenger));
#endif
    }

    void VulkanContext::createFrameData() {
        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            frameDatas[i].imageAvailableSemaphore.init(device);
            frameDatas[i].renderFinishedSemaphore.init(device);
            frameDatas[i].renderFinishedFence.init(device, true);

            for(const auto &fam : physicalDevice.getQueueFamilyContainer()) {
                if(fam.isInUse())
                    frameDatas[i].commandPoolsByFamily[fam.getIndex()].init(device, fam);
            }
        }
    }

    void VulkanContext::cleanupFrameData() {
        for(uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            frameDatas[i].imageAvailableSemaphore.destroy();
            frameDatas[i].renderFinishedSemaphore.destroy();
            frameDatas[i].renderFinishedFence.destroy();

            for(auto &pair : frameDatas[i].commandPoolsByFamily) {
                pair.second.destroy();
            }
        }
    }

    template<typename T>
    static T VulkanContext::getExtensionFunction(VkInstance instance, const char *name) {
        auto func = (T) vkGetInstanceProcAddr(instance, name);
        BZ_CRITICAL_ERROR_CORE(func, "Unable to get {} function pointer!", name);
        return func;
    }

    void VulkanContext::assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const {
        uint32_t layerCount;
        BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        
        std::vector<VkLayerProperties> availableLayers(layerCount);
        BZ_ASSERT_VK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

        for(const char* layerName : requiredLayers) {
            bool layerFound = false;
            for(const auto& layerProperties : availableLayers) {
                if(strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            BZ_CRITICAL_ERROR_CORE(layerFound, "Requested Validation Layer '{}' but it was not found!", layerName);
        }
    }

    Ref<CommandBuffer> VulkanContext::getCurrentFrameCommandBuffer() {
       return getCurrentFrameCommandPool(QueueProperty::Graphics, false).getCommandBuffer();
    }

    void VulkanContext::beginFrame() {
        //Make sure that this frame has finished before reutilizing its data. If not, we are GPU bound and should block here.
        frameDatas[currentFrameIndex].renderFinishedFence.waitFor();
        frameDatas[currentFrameIndex].renderFinishedFence.reset();

        FrameData &frameData = frameDatas[currentFrameIndex];
        for (auto &familyAndPool : frameData.commandPoolsByFamily) {
            familyAndPool.second.reset();
        }

        swapchain.aquireImage(frameDatas[currentFrameIndex].imageAvailableSemaphore);
    }

    void VulkanContext::submitCommandBuffersAndFlush(const Ref<CommandBuffer> commandBuffers[], uint32 count) {
        VkCommandBuffer vkCommandBuffers[MAX_COMMAND_BUFFERS];
        uint32 idx;
        for(idx = 0; idx < count; ++idx) {
            auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffers[idx]);
            vkCommandBuffers[idx] = vulkanCommandBuffer.getNativeHandle();
        }

        VkSemaphore waitSemaphores[] = { frameDatas[currentFrameIndex].imageAvailableSemaphore.getNativeHandle() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        VkSemaphore signalSemaphores[] = { frameDatas[currentFrameIndex].renderFinishedSemaphore.getNativeHandle() };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = idx;
        submitInfo.pCommandBuffers = vkCommandBuffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        BZ_ASSERT_VK(vkQueueSubmit(device.getQueueContainer().graphics.getNativeHandle(), 1, &submitInfo, frameDatas[currentFrameIndex].renderFinishedFence.getNativeHandle()));

        swapchain.presentImage(frameDatas[currentFrameIndex].renderFinishedSemaphore);
        currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanContext::waitForDevice() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));
    }
}