#pragma once

#include "Bhazel/Renderer/RendererApi.h"


namespace BZ {

    class VulkanContext;

    class VulkanRendererAPI : public RendererApi {
    public:
        VulkanRendererAPI(VulkanContext &graphicsContext);

        Ref<CommandBuffer> startRecording() override;
        Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex) override;

    private:
        VulkanContext &graphicsContext;
    };
}