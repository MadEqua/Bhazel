#pragma once


namespace BZ {

class RenderPass;
class Framebuffer;
class Event;
class DescriptorSet;

/*
 * Coordinates Renderers accessing the final destination Framebuffer (either Swapchain or an offscreen Framebuffer).
 * Controls the layout transitions, clears/loads and frame Semaphore/Fence signaling.
 */
class RendererCoordinator {
  public:
    BZ_NON_COPYABLE(RendererCoordinator);
    
    RendererCoordinator() = default;

    void init();
    void destroy();

    void enable3dRenderer(bool enable);
    void enable2dRenderer(bool enable);
    void enableImGuiRenderer(bool enable);

    // Force offscreen rendering for 3D and 2D renderers. Useful for tools like the editor.
    void forceOffscreenRendering(bool force);

    void onEvent(Event &ev);
    void render();

    DescriptorSet *getOffscreenTextureDescriptorSet() { return offscreenTextureDescriptorSet; }

  private:
    bool is3dActive;
    bool is2dActive;
    bool isImGuiActive;
    bool isForceOffscreenRendering;

    Ref<RenderPass> firstPass;
    Ref<RenderPass> secondPass;
    Ref<RenderPass> lastPass;
    Ref<RenderPass> firstAndLastPass;

    // Swapchain replica used when isForceOffscreenRendering is true. Lazy initialized.
    Ref<Framebuffer> offscreenFramebuffer;
    Ref<RenderPass> firstPassOffscreen;
    Ref<RenderPass> secondPassOffscreen;
    Ref<RenderPass> lastPassOffscreen;
    Ref<RenderPass> firstAndLastPassOffscreen;

    DescriptorSet *offscreenTextureDescriptorSet;

    uint32 getActiveRendererCount() const;
};
}