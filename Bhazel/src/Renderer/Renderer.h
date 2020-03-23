#pragma once

#include "Camera.h"
#include "Graphics/Texture.h"


namespace BZ {

    //TODO
    class Mesh {
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

