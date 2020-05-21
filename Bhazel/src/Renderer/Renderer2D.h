#pragma once


namespace BZ {

    class ParticleSystem2D;
    class OrthographicCamera;
    class Texture2D;
    struct FrameStats;
    class RenderPass;
    class Framebuffer;

    struct Sprite {
        glm::vec2 position;
        glm::vec2 dimensions;
        float rotationDeg;
        glm::vec4 tintAndAlpha;
        Ref<Texture2D> texture;
    };


    /*
    * Batch renderer for 2D geometry.
    */
    class Renderer2D {
    public:
        static void begin(const OrthographicCamera &camera);
        static void end();

        static void renderSprite(const Sprite &sprite);

        static void renderQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec4 &colorAndAlpha);
        static void renderQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec4 &tintAndAlpha);

        static void renderParticleSystem2D(const ParticleSystem2D &particleSystem);

    private:
        friend class RendererCoordinator;
        friend class Application;

        static void init();
        static void destroy();

        static void render(const Ref<RenderPass> &swapchainRenderPass, const Ref<Framebuffer> &swapchainFramebuffer, bool waitForImageAvailable, bool signalFrameEnd);
        static void onImGuiRender(const FrameStats &frameStats);
    };
}

