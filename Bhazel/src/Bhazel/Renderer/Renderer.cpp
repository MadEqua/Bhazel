#include "bzpch.h"

#include "Renderer.h"
#include "OrtographicCamera.h"

#include "Bhazel/Platform/OpenGL/OpenGLShader.h"


namespace BZ {

    Renderer::FrameData Renderer::sceneData;
    Renderer::InstanceData Renderer::instanceData;

    Ref<ConstantBuffer> Renderer::frameConstantBuffer;
    Ref<ConstantBuffer> Renderer::instanceConstantBuffer;

    void Renderer::init() {
        frameConstantBuffer = ConstantBuffer::create(sizeof(sceneData));
        instanceConstantBuffer = ConstantBuffer::create(sizeof(instanceData));
    }

    void Renderer::destroy() {
        //Destroy this 'manually' to avoid the static destruction lottery
        frameConstantBuffer.reset();
        instanceConstantBuffer.reset();
    }

    void Renderer::beginScene(OrtographicCamera &camera) {
        sceneData.viewMatrix = camera.getViewMatrix();
        sceneData.projectionMatrix = camera.getProjectionMatrix();
        sceneData.viewProjectionMatrix = camera.getViewProjectionMatrix();

        frameConstantBuffer->setData(&sceneData, sizeof(sceneData));
        frameConstantBuffer->bindToPipeline(0);
    }

    void Renderer::endScene() {
    }

    void Renderer::submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix) {
        instanceData.modelMatrix = modelMatrix;
        instanceConstantBuffer->setData(&instanceData, sizeof(instanceData));
        instanceConstantBuffer->bindToPipeline(1);

        shader->bindToPipeline();

        inputDescription->bindToPipeline();
        RenderCommand::drawIndexed(inputDescription);
    }
}