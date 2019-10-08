#pragma once


namespace BZ {

    constexpr static int MAX_FRAMES_IN_FLIGHT = 8;

    class Camera;
    class InputDescription;
    class Buffer;
    class Shader;
    struct FrameStats;
    class WindowResizedEvent;
    class CommandBuffer;
    class FrameBuffer;
    class PipelineState;
    class DescriptorSet;
    class RendererApi;

    class Renderer {
    public:

        enum class API {
            Unknown,
            OpenGL,
            D3D11,
            Vulkan
        };

        static void init();
        static void destroy();

        static void onWindowResize(WindowResizedEvent &ev);

        //Start recording getting a CommandBuffer from the CommandPool reserved to the current frame
        static Ref<CommandBuffer> startRecording();

        //Start recording getting a CommandBuffer from the CommandPool reserved to the frame 'frameIndex'
        //Useful to record static buffers only once and reutilize them every frame
        static Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex);

        //static void bindFramebuffer(const Ref<FrameBuffer> &framebuffer);
        //static void bindPipelineState(const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSet(const Ref<DescriptorSet> &descriptorSet);
        //
        //static void draw(const Ref<Buffer> &vertexBuffer, uint32 instances = 1, const glm::mat4 &modelMatrix = glm::mat4(1.0f));
        //static void drawIndexed(const Ref<Buffer> &vertexBuffer, const Ref<Buffer> &indexBuffer, uint32 instances = 1, const glm::mat4 &modelMatrix = glm::mat4(1.0f));
        //
        //static Ref<CommandBuffer> endRecording();
        //
        //static void submit(const Camera &camera, const FrameStats &frameStats, const Ref<CommandBuffer> &commandBuffer);

        static API api;

    private:
        struct ConstantBufferData {
            //Per frame
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) glm::vec2 timeAndDelta;

            //Per instance
            glm::mat4 modelMatrix;
        };
        static ConstantBufferData constantBufferData;
        static Ref<Buffer> constantBuffer;

        static RendererApi *rendererApi;
    };
}