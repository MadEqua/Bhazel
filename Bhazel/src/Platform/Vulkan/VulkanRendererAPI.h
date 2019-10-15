#pragma once

#include "Graphics/GraphicsApi.h"


namespace BZ {

    class VulkanContext;


    class VulkanRendererApi : public GraphicsApi {
    public:
        VulkanRendererApi(VulkanContext &graphicsContext);

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

    private:
        VulkanContext &graphicsContext;
    };
}