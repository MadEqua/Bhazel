#pragma once

#include "Camera.h"
#include "Graphics/Texture.h"


namespace BZ {

    class DescriptorSet;

    class Renderer2D {
    public:
        static void beginScene(const OrthographicCamera &camera);
        static void endScene();

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const glm::vec3 &color);
        static void drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, float rotationDeg, const Ref<Texture2D> &texture, const glm::vec3 &tint);

    private:
        friend class Application;

        static void init();
        static void destroy();

        static const Ref<DescriptorSet>& getDescriptorSetForTexture(const Ref<Texture2D>& texture);
    };
}

