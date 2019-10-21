#pragma once

#include "Graphics/GraphicsContext.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanDevice.h"
#include "Platform/Vulkan/Internal/VulkanSwapchain.h"
#include "Platform/Vulkan/Internal/VulkanSurface.h"
#include "Platform/Vulkan/Internal/VulkanSync.h"
#include "Platform/Vulkan/Internal/VulkanCommandPool.h"
#include "Platform/Vulkan/Internal/VulkanDescriptorPool.h"

#include "Graphics/Graphics.h"

struct GLFWwindow;


namespace BZ {

    class Framebuffer;

    class VulkanContext : public GraphicsContext {
    public:
        explicit VulkanContext(void *windowHandle);
        ~VulkanContext() override;

        void init() override;
        void onWindowResize(WindowResizedEvent &e) override;
        void presentBuffer() override;

        void setVSync(bool enabled) override;

        VkDevice getDevice() const { return device.getNativeHandle(); }
        //VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        uint32 getCurrentFrame() const { return currentFrame; }
        VulkanCommandPool& getCommandPool(QueueProperty property, uint32 frame, bool exclusive);
        VulkanDescriptorPool& getDescriptorPool() { return descriptorPool; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        /////////////////////////////////////////////////////////
        // API
        /////////////////////////////////////////////////////////
        Ref<CommandBuffer> startRecording() override;
        Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer) override;

        Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex) override;
        Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) override;

        void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) override;
        void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) override;

        void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) override;
        //void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet) override;
        void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) override;

        void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) override;
        void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;

        void endRecording(const Ref<CommandBuffer> &commandBuffer) override;

        void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) override;
        void endFrame() override;

        void waitForDevice() override;

    private:
        VkInstance instance;
        GLFWwindow *windowHandle;

        VulkanPhysicalDevice physicalDevice;
        VulkanDevice device;
        VulkanSurface surface;
        VulkanSwapchain swapchain;

        struct FrameData {
            //One command pool per frame makes it easy to reset all the allocated buffers on frame end. No need to track anything else.
            std::unordered_map<uint32, VulkanCommandPool> commandPoolsByFamily;
            VulkanSemaphore imageAvailableSemaphore;
            VulkanSemaphore renderFinishedSemaphore;
            VulkanFence inFlightFence;
        };
        FrameData frameData[MAX_FRAMES_IN_FLIGHT];
        uint32 currentFrame = 0;

        VulkanDescriptorPool descriptorPool;

#ifndef BZ_DIST
        VkDebugUtilsMessengerEXT debugMessenger;
#endif

        void createInstance();

        void createFrameData();
        void cleanupFrameData();

        template<typename T>
        static T getExtensionFunction(VkInstance instance, const char *name);

        void assertValidationLayerSupport(const std::vector<const char*> &requiredLayers) const;
    };
}