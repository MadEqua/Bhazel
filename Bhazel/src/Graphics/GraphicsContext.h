#pragma once


namespace BZ {

    class WindowResizedEvent;
    class Framebuffer;
    class CommandBuffer;

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
        virtual Ref<CommandBuffer> getCurrentFrameCommandBuffer() = 0;

        virtual void submitCommandBuffersAndFlush(const Ref<CommandBuffer> commandBuffers[], uint32 count) = 0;
        virtual void waitForDevice() = 0;

    protected:
        GraphicsContext() = default;

        bool vsync = true;
    };
}