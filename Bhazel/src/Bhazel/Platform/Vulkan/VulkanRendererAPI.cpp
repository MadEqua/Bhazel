#include "bzpch.h"

#include "VulkanRendererApi.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanContext.h"
#include "Bhazel/Platform/Vulkan/VulkanCommandBuffer.h"


namespace BZ {

    VulkanRendererAPI::VulkanRendererAPI(VulkanContext &graphicsContext) :
        graphicsContext(graphicsContext) {
    }

    Ref<CommandBuffer> VulkanRendererAPI::startRecording() {
        return startRecordingForFrame(graphicsContext.getCurrentFrame());
    }

    Ref<CommandBuffer> VulkanRendererAPI::startRecordingForFrame(uint32 frameIndex) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        auto &commandBufferRef = VulkanCommandBuffer::create(RenderQueueFamily::Graphics, frameIndex);
        BZ_ASSERT_VK(vkBeginCommandBuffer(commandBufferRef->getNativeHandle(), &beginInfo));
        return commandBufferRef;
    }
}