#pragma once


namespace BZ {

    class Buffer;
    class WindowResizedEvent;
    class CommandBuffer;
    class Framebuffer;
    class PipelineState;
    class DescriptorSet;
    class DescriptorSetLayout;
    struct Viewport;
    struct ScissorRect;

    class GraphicsContext {
    public:
        static GraphicsContext* create(void *windowHandle);

        virtual void init() = 0;

        virtual ~GraphicsContext() = default;
        
        virtual void onWindowResize(WindowResizedEvent& e) {};

        virtual void setVSync(bool enabled) { vsync = enabled; };
        bool isVSync() const { return vsync; }

        virtual uint32 getCurrentFrameIndex() const = 0;
        virtual Ref<Framebuffer> getCurrentFrameFramebuffer() = 0;

        /////////////////////////////////////////////////////////
        // API
        /////////////////////////////////////////////////////////
        virtual Ref<CommandBuffer> startRecording() = 0;
        virtual Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer) = 0;

        virtual void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) = 0;
        virtual void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset) = 0;

        virtual void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState) = 0;
        //virtual void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet) = 0;
        virtual void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet,
                                       const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                       uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) = 0;

        virtual void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
        virtual void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;

        //Pipeline dynamic state changes
        virtual void setViewports(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount) = 0;
        virtual void setScissorRects(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount) = 0;

        virtual void endRecording(const Ref<CommandBuffer> &commandBuffer) = 0;

        virtual void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer) = 0;
        virtual void endFrame() = 0;

        virtual void waitForDevice() = 0;

    protected:
        GraphicsContext() = default;

        bool vsync = true;
    };
}