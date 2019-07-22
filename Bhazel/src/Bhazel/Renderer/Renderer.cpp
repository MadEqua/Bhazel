#include "bzpch.h"

#include "Renderer.h"
#include "OrtographicCamera.h"


namespace BZ {

    Renderer::SceneData *Renderer::sceneData = new Renderer::SceneData();

    void Renderer::beginScene(OrtographicCamera &camera) {
        sceneData->viewProjection = camera.getViewProjectionMatrix();
    }

    void Renderer::endScene() {
    }

    void Renderer::submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform) {
        shader->bind();
        shader->setUniformMat4("viewProjectionMatrix", sceneData->viewProjection);
        shader->setUniformMat4("modelMatrix", transform);

        vertexArray->bind();
        RenderCommand::drawIndexed(vertexArray);
    }
}