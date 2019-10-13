#pragma once


namespace BZ {

    class Buffer;
    class CommandBuffer;
    class Framebuffer;
    class PipelineState;
    class DescriptorSet;
    class RendererApi;


    class RendererApi {
    public:
        virtual ~RendererApi() = default;

        virtual Ref<CommandBuffer> startRecording() = 0;
        virtual Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer) = 0;

        virtual Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex) = 0;
        virtual Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer) = 0;
        
        virtual void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) = 0;
        virtual void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer) = 0;

        virtual void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) = 0;
        //virtual void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet) = 0;
        virtual void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState) = 0;

        virtual void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
        virtual void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;

        virtual void endRecording(const Ref<CommandBuffer> &commandBuffer) = 0;

        virtual void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) = 0;
        virtual void endFrame() = 0;



        //virtual void setClearColor(const glm::vec4 &color) = 0;
        //virtual void clearColorBuffer() = 0;
        //virtual void clearDepthBuffer() = 0;
        //virtual void clearStencilBuffer() = 0;
        //virtual void clearColorAndDepthStencilBuffers() = 0;
        //
        ////virtual void setBlendingSettings(BlendingSettings &settings) = 0;
        ////virtual void setDepthSettings(DepthSettings &settings) = 0;
        ////virtual void setStencilSettings() = 0;
        //
        ////virtual void setBackfaceCullingSettings();
        //
        //virtual void setViewport(int left, int top, int width, int height) = 0;
        //
        ////virtual void setRenderMode(Renderer::PrimitiveTopology mode) = 0;
        //
        //virtual void draw(uint32 vertexCount) = 0;
        //virtual void drawIndexed(uint32 indicesCount) = 0;
        //virtual void drawInstanced(uint32 vertexCount, uint32 instanceCount) = 0;
        //virtual void drawInstancedIndexed(uint32 indicesCount, uint32 instanceCount) = 0;
        //
        //virtual void submitCompute(uint32 groupsX, uint32 groupsY, uint32 groupsZ) = 0;
    };
}