#pragma once


namespace BZ {

    constexpr static int MAX_FRAMES_IN_FLIGHT = 3;
    constexpr static int MAX_FRAMEBUFFER_ATTACHEMENTS = 8;

    class Buffer;
    class WindowResizedEvent;
    class CommandBuffer;
    class Framebuffer;
    class PipelineState;
    class DescriptorSet;
    class GraphicsApi;

    class Graphics {
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

        //Start recording getting a CommandBuffer from the CommandPool reserved to the current frame.
        static Ref<CommandBuffer> startRecording();
        static Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer);

        //Start recording getting a CommandBuffer from the CommandPool reserved to the frame 'frameIndex'
        //Useful to record static buffers only once and reutilize them every frame
        static Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex);
        static Ref<CommandBuffer> startRecordingForFrame(uint32 frameIndex, const Ref<Framebuffer> &framebuffer);

        static void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer);
        static void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer);
        
        static void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        static void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState);

        static void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        static void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        static void endRecording(const Ref<CommandBuffer> &commandBuffer);

        static void startFrame();
        static void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer);
        static void endFrame();

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

        static GraphicsApi *graphicsApi;
    };
}