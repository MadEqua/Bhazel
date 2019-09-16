#pragma once

#include "Bhazel/Core/Timer.h"


namespace BZ {

    class OrtographicCamera;
    class InputDescription;
    class Buffer;
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
            Triangles,
            LineStrip,
            TriangleStrip
        };

        static void init();
        static void destroy();

        static void beginScene(OrtographicCamera &camera);
        static void endScene();

        static void submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix = glm::mat4(1.0f), RenderMode renderMode = RenderMode::Triangles);

        static void submitCompute(const Ref<Shader> &computeShader, uint32 groupsX, uint32 groupsY, uint32 groupsZ, std::initializer_list<Ref<Buffer>> buffers);

        static API api;
    private:
        struct alignas(16) FrameData {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            float runningTime;
        };
        static FrameData frameData;

        struct alignas(16) InstanceData {
            glm::mat4 modelMatrix;
        };
        static InstanceData instanceData;

        static Ref<Buffer> frameConstantBuffer;
        static Ref<Buffer> instanceConstantBuffer;

        static Timer timer;
    };
}