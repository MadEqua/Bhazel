#pragma once

#include "Graphics/GraphicsContext.h"


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

    // TODO: make this private and Engine friend.
    RendererCoordinator() = default;

    void init(bool enable2dRenderer, bool enable3dRenderer, bool enableImGuiRenderer);
    void initEditorMode();
    void destroy();

    void onEvent(Event &ev);
    void render();

    DescriptorSet *getOffscreenTextureDescriptorSet();

  private:
    Ref<Framebuffer> offscreenFramebuffers[GraphicsContext::MAX_FRAMES_IN_FLIGHT];
    DescriptorSet *offscreenTextureDescriptorSets[GraphicsContext::MAX_FRAMES_IN_FLIGHT];

    Ref<RenderPass> firstPass;
    Ref<RenderPass> secondPass;
    Ref<RenderPass> lastPass;
    Ref<RenderPass> firstAndLastPass;

    Ref<RenderPass> lastPassEditorMode;

    std::function<void()> renderFunction;

    void internalInit();
};
}