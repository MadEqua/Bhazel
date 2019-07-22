#pragma once

#include <memory>

#include "RenderCommand.h"
#include "Shader.h"


namespace BZ {

    class OrtographicCamera;

    class Renderer
    {
    public:
        static void beginScene(OrtographicCamera &camera);
        static void endScene();

        static void submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform = glm::mat4(1.0f));

        static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }

    private:
        struct SceneData {
            glm::mat4 viewProjection;
        };
        static SceneData *sceneData;
    };
}