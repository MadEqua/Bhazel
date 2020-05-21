#pragma once


namespace BZ {

    class Event;
    class RenderPass;
    class Framebuffer;

    class RendererImGui {
    public:
        static void begin();
        static void end();

        static void onEvent(Event &event);

    private:
        friend class RendererCoordinator;
        friend class Application;

        static void init();
        static void destroy();

        static void initInput();
        static void initGraphics();

        static void render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer, bool waitForImageAvailable, bool signalFrameEnd);
    };
}