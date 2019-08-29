#pragma once

#include "RenderCommand.h"
#include "Shader.h"


namespace BZ {

    class OrtographicCamera;

    enum class RenderMode {
        Points,
        Lines,
        Triangles
    };

    class Renderer
    {
    public:
        static void init();

        static void beginScene(OrtographicCamera &camera);
        static void endScene();

        static void submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix = glm::mat4(1.0f));

        static RendererAPI::API getAPI() { return RendererAPI::getAPI(); }

    private:
        struct FrameData {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
        };
        static FrameData sceneData;

        struct InstanceData {
            glm::mat4 modelMatrix;
        };
        static InstanceData instanceData;

        static Ref<ConstantBuffer> frameConstantBuffer;
        static Ref<ConstantBuffer> instanceConstantBuffer;
    };
}