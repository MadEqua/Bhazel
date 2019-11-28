#pragma once

#include "Camera.h"
#include "Graphics/Texture.h"


namespace BZ {

    class DescriptorSet;

    struct Sprite {
        glm::vec2 position;
        glm::vec2 dimensions;
        float rotationDeg;
        glm::vec3 tint;
        Ref<Texture2D> texture;
    };

    struct Renderer2DStats {
        uint32 spriteCount;
        uint32 drawCallCount;
        uint32 descriptorSetBindCount;
        uint32 tintPushCount;
    };

    class Renderer2D {
    public:
        static void beginScene(const OrthographicCamera &camera);
        static void endScene();

        static void drawSprite(const Sprite &sprite);

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec3 &color);
        static void drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec3 &tint);

        static const Renderer2DStats& getStats() { return stats; }

    private:
        friend class Application;

        static void init();
        static void destroy();

        static Renderer2DStats stats;
    };
}

