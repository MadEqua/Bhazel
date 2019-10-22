#include "bzpch.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"

#include <GLFW/glfw3.h>


namespace BZ {

    VulkanContext::VulkanContext(void *windowHandle) :
        windowHandle(static_cast<GLFWwindow*>(windowHandle)) {
        BZ_ASSERT_CORE(windowHandle, "Window handle is null!");
    }

    VulkanContext::~VulkanContext() {
        descriptorPool.destroy();
        cleanupFrameData();
        swapchain.destroy();
        surface.destroy();
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

        createFrameData();

        swapchain.init(device, surface);
        swapchain.aquireImage(frameDatas[currentFrame].imageAvailableSemaphore);

        VulkanDescriptorPool::Builder builder;
        builder.addDescriptorTypeCount(DescriptorType::ConstantBuffer, 1024);
        descriptorPool.init(device, builder);
    }

    void VulkanContext::presentBuffer() {
        swapchain.presentImage(frameDatas[currentFrame].renderFinishedSemaphore);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        swapchain.aquireImage(frameDatas[currentFrame].imageAvailableSemaphore);

        FrameData &frameData = frameDatas[currentFrame];
        for(auto &familyAndPool : frameData.commandPoolsByFamily) {
            if(frameData.renderFinishedFence.isSignaled()) //TODO: is we are gpu bound and the fence is never signaled here, then the pool will never be reset
                familyAndPool.second.reset();
            else
                BZ_LOG_DEBUG("fence is not signaled!");
        }
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
        return frameDatas[currentFrame].commandPoolsByFamily[families[0]->getIndex()];
    }

    uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        return physicalDevice.findMemoryType(typeFilter, properties);
    }

    void VulkanContext::onWindowResize(WindowResizedEvent& e) {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));

        swapchain.recreate();
        cleanupFrameData();
        createFrameData();

        currentFrame = 0;
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
        BZ_ASSERT_CORE(func, "Unable to get {} function pointer!", name);
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

            if(!layerFound)
                BZ_ASSERT_ALWAYS_CORE("Requested Validation Layer '{}' but it was not found!", layerName);
        }
    }

    /////////////////////////////////////////////////////////
    // API
    /////////////////////////////////////////////////////////
    Ref<CommandBuffer> VulkanContext::startRecording() {
        return startRecording(swapchain.getFramebuffer(currentFrame));
    }

    Ref<CommandBuffer> VulkanContext::startRecording(const Ref<Framebuffer> &framebuffer) {
        //Begin a command buffer
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //Disallowing command buffer reusage
        beginInfo.pInheritanceInfo = nullptr;

        auto &commandBufferRef = VulkanCommandBuffer::create(QueueProperty::Graphics, currentFrame);
        BZ_ASSERT_VK(vkBeginCommandBuffer(commandBufferRef->getNativeHandle(), &beginInfo));

        //Record a render pass
        auto &vulkanFramebuffer = static_cast<const VulkanFramebuffer &>(*framebuffer);

        //We know that the color attachments will be first and then the depthstencil
        VkClearValue clearValues[MAX_FRAMEBUFFER_ATTACHEMENTS];
        int i;
        for(i = 0; i < vulkanFramebuffer.getColorAttachmentCount(); ++i) {
            auto &attachmentClearValues = vulkanFramebuffer.getColorAttachment(i).description.clearValues;
            memcpy(clearValues[i].color.float32, &attachmentClearValues, sizeof(float) * 4);
        }
        if(vulkanFramebuffer.hasDepthStencilAttachment()) {
            auto &attachmentClearValues = vulkanFramebuffer.getDepthStencilAttachment()->description.clearValues;
            clearValues[i].depthStencil.depth = attachmentClearValues.floating.x;
            clearValues[i].depthStencil.stencil = attachmentClearValues.integer.y;
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanFramebuffer.getNativeHandle().renderPassHandle;
        renderPassBeginInfo.framebuffer = vulkanFramebuffer.getNativeHandle().frameBufferHandle;
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(vulkanFramebuffer.getDimensions().x), static_cast<uint32_t>(vulkanFramebuffer.getDimensions().y) };
        renderPassBeginInfo.clearValueCount = vulkanFramebuffer.getAttachmentCount();
        renderPassBeginInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(commandBufferRef->getNativeHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        return commandBufferRef;
    }

    /*Ref<CommandBuffer> VulkanContext::startRecordingForFrame(uint32 frameIndex) {
        return startRecordingForFrame(frameIndex, swapchain.getFramebuffer(frameIndex));
    }

    Ref<CommandBuffer> VulkanContext::startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) {
    }*/

    void VulkanContext::bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        VkBuffer vkBuffers[] = { static_cast<const VulkanBuffer &>(*buffer).getNativeHandle() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(vulkanCommandBuffer.getNativeHandle(), 0, 1, vkBuffers, offsets);
    }

    void VulkanContext::bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanBuffer = static_cast<const VulkanBuffer &>(*buffer);
        vkCmdBindIndexBuffer(vulkanCommandBuffer.getNativeHandle(), vulkanBuffer.getNativeHandle(), 0, VK_INDEX_TYPE_UINT16); //TODO index size
    }

    void VulkanContext::bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        vkCmdBindPipeline(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineState.getNativeHandle().pipeline);
    }

    void VulkanContext::bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        auto &vulkanPipelineState = static_cast<const VulkanPipelineState &>(*pipelineState);
        VkDescriptorSet descSets[] = { static_cast<const VulkanDescriptorSet &>(*descriptorSet).getNativeHandle() };
        vkCmdBindDescriptorSets(vulkanCommandBuffer.getNativeHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
            vulkanPipelineState.getNativeHandle().pipelineLayout, 0, 1, descSets, 0, nullptr);
    }

    void VulkanContext::draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDraw(vulkanCommandBuffer.getNativeHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanContext::drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdDrawIndexed(vulkanCommandBuffer.getNativeHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanContext::endRecording(const Ref<CommandBuffer> &commandBuffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);
        vkCmdEndRenderPass(vulkanCommandBuffer.getNativeHandle());
        BZ_ASSERT_VK(vkEndCommandBuffer(vulkanCommandBuffer.getNativeHandle()));
    }

    void VulkanContext::submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) {
        auto &vulkanCommandBuffer = static_cast<const VulkanCommandBuffer &>(*commandBuffer);

        VkSemaphore waitSemaphores[] = { frameDatas[currentFrame].imageAvailableSemaphore.getNativeHandle() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        VkCommandBuffer vkCommandBuffers[] = { vulkanCommandBuffer.getNativeHandle() };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = vkCommandBuffers;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        frameDatas[currentFrame].renderFinishedFence.waitFor();

        BZ_ASSERT_VK(vkQueueSubmit(device.getQueueContainer().graphics.getNativeHandle(), 1, &submitInfo, VK_NULL_HANDLE));
    }

    void VulkanContext::endFrame() {
        VkSemaphore signalSemaphores[] = { frameDatas[currentFrame].renderFinishedSemaphore.getNativeHandle() };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 0;
        submitInfo.pCommandBuffers = nullptr;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        frameDatas[currentFrame].renderFinishedFence.waitFor();
        frameDatas[currentFrame].renderFinishedFence.reset();

        BZ_ASSERT_VK(vkQueueSubmit(device.getQueueContainer().graphics.getNativeHandle(), 1, &submitInfo, frameDatas[currentFrame].renderFinishedFence.getNativeHandle()));
    }

    void VulkanContext::waitForDevice() {
        BZ_ASSERT_VK(vkDeviceWaitIdle(device.getNativeHandle()));
    }
}