#include "bzpch.h"

#include "Renderer.h"
#include "OrtographicCamera.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"


namespace BZ {

    Renderer::SceneData *Renderer::sceneData = new Renderer::SceneData();

    void Renderer::beginScene(OrtographicCamera &camera) {
        sceneData->viewProjection = camera.getViewProjectionMatrix();
    }

    void Renderer::endScene() {
    }

    void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &transform) {
        shader->bind();
        std::static_pointer_cast<OpenGLShader>(shader)->setUniformMat4("modelMatrix", transform);
        std::static_pointer_cast<OpenGLShader>(shader)->setUniformMat4("viewProjectionMatrix", sceneData->viewProjection);

        inputDescription->bind();
        RenderCommand::drawIndexed(inputDescription);
    }
}