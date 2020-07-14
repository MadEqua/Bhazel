#pragma once


namespace BZ {

class ParticleSystem2D;
class Texture2D;
struct FrameTiming;
class RenderPass;
class Framebuffer;
class Scene;

struct Sprite {
    glm::vec2 position = {};
    glm::vec2 dimensions = { 10.0f, 10.0f };
    float rotationDeg = 0.0f;
    glm::vec4 tintAndAlpha = { 1.0f, 1.0f, 1.0f, 1.0f };
    Ref<Texture2D> texture;
};


/*
 * Batch renderer for 2D geometry.
 */
class Renderer2D {
  private:
    friend class RendererCoordinator;
    friend class Engine;

    static void init();
    static void destroy();

    static void addSprite(const Sprite &sprite);

    static void addQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg,
                        const glm::vec4 &colorAndAlpha);
    static void addQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg,
                        const Ref<Texture2D> &texture, const glm::vec4 &tintAndAlpha);

    static void addParticleSystem2D(const ParticleSystem2D &particleSystem);

    static void render(const Scene &scene, const Ref<RenderPass> &finalRenderPass,
                       const Ref<Framebuffer> &finalFramebuffer, bool waitForImageAvailable, bool signalFrameEnd);
    static void onImGuiRender(const FrameTiming &frameTiming);
};
}
