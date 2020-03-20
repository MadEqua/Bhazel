#pragma once

#include "Camera.h"
#include "Graphics/Texture.h"


namespace BZ {

    class Mesh {
    };

    class Transform {
        glm::vec3 translation = {};
        glm::vec3 rotationEuler = {};
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    };

    struct RendererStats {
        uint32 vertexCount;
        uint32 drawCallCount;
        uint32 descriptorSetBindCount;
    };

    class Renderer {
    public:
        static void beginScene(const Camera &camera);
        static void endScene();

        static void drawCube(const Transform &transform); //TODO: material?

        static void drawMesh(const Mesh &mesh);

        static const RendererStats& getStats() { return stats; }

    private:
        friend class Application;

        static void init();
        static void destroy();

        static RendererStats stats;
    };
}

