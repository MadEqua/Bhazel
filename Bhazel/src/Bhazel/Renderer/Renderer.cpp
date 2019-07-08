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

    void Renderer::submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray) {
        shader->bind();
        shader->setUniformMat4("viewProjection", sceneData->viewProjection);

        vertexArray->bind();
        RenderCommand::drawIndexed(vertexArray);
    }
}