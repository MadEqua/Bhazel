#pragma once


namespace BZ {

    class OrtographicCamera;
    class InputDescription;
    class ConstantBuffer;
    class Shader;

    class Renderer
    {
    public:

        enum class API {
            Unknown,
            OpenGL,
            D3D11
        };

        enum class RenderMode {
            Points,
            Lines,
            Triangles
        };

        static void init();
        static void destroy();

        static void beginScene(OrtographicCamera &camera);
        static void endScene();

        static void submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix = glm::mat4(1.0f));

        static API api;
    private:
        struct FrameData {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
        };
        static FrameData frameData;

        struct InstanceData {
            glm::mat4 modelMatrix;
        };
        static InstanceData instanceData;

        static Ref<ConstantBuffer> frameConstantBuffer;
        static Ref<ConstantBuffer> instanceConstantBuffer;
    };
}