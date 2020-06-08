#pragma once


namespace BZ {

class RenderPass;
class Event;

/*
 * Coordinates Renderers accessing the Swapchain Framebuffers (layout transitions and frame Semaphore/Fence handling).
 */
class RendererCoordinator {
  public:
    void init();
    void destroy();

    void enable3dRenderer(bool enable);
    void enable2dRenderer(bool enable);
    void enableImGuiRenderer(bool enable);

    void onEvent(Event &ev);
    void render();

  private:
    bool is3dActive;
    bool is2dActive;
    bool isImGuiActive;

    Ref<RenderPass> firstPass;
    Ref<RenderPass> secondPass;
    Ref<RenderPass> lastPass;
    Ref<RenderPass> firstAndLastPass;

    uint32 getActiveRendererCount() const;
};
}