#pragma once


namespace BZ {

    class Camera;
    class InputDescription;
    class Buffer;
    class Shader;
    struct FrameStats;
    class WindowResizedEvent;

    class Renderer {
    public:

        enum class API {
            Unknown,
            OpenGL,
            D3D11,
            Vulkan
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

        static void onWindowResize(WindowResizedEvent &ev);

        static void beginScene(Camera &camera, const FrameStats &frameStats);
        static void endScene();

        static void submit(const Ref<Shader> &shader, const Ref<InputDescription> &inputDescription, const glm::mat4 &modelMatrix = glm::mat4(1.0f), RenderMode renderMode = RenderMode::Triangles, uint32 instances = 1);
        static void submitCompute(const Ref<Shader> &computeShader, uint32 groupsX, uint32 groupsY, uint32 groupsZ, std::initializer_list<Ref<Buffer>> buffers);

        static API api;
    private:
        struct FrameData {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) glm::vec2 timeAndDelta;
        };
        static FrameData frameData;

        struct InstanceData {
            glm::mat4 modelMatrix;
        };
        static InstanceData instanceData;

        static Ref<Buffer> frameConstantBuffer;
        static Ref<Buffer> instanceConstantBuffer;
    };
}