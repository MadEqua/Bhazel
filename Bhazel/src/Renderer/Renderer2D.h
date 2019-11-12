#pragma once

#include "Camera.h"


namespace BZ {

    class Renderer2D {
    public:
        static void beginScene(const OrthographicCamera &camera);
        static void endScene();

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &dimensions, const glm::vec3 &tint);
        //static void drawQuad(const glm::vec3 &position, const glm::vec2 &dimensions, const glm::vec4 &tint);

    private:
        friend class Application;

        static void init();
        static void destroy();
    };
}

