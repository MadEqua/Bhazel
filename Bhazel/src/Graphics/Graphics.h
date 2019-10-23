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
    class DescriptorSetLayout;
    class GraphicsContext;

    /*
    * Low level Graphics API. Static wrapper to the GraphicsContext API.
    * Also manages data related to engine ConstantBuffers.
    */
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

        static void startScene(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix); //TODO: more frameData
        static void startObject(const glm::mat4 &modelMatrix);

        static void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer);
        static void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer);
        
        static void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        static void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, const Ref<PipelineState> &pipelineState);

        static void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        static void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        static void endRecording(const Ref<CommandBuffer> &commandBuffer);

        static void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer);
        static void endFrame();

        static void waitForDevice();

        static Ref<DescriptorSetLayout> getDescriptorSetLayout() { return descriptorSetLayout; }

        static API api;

    private:
        struct ConstantBufferData {
            //Per frame
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            alignas(16) glm::vec3 cameraPosition;
            alignas(8) glm::vec2 timeAndDelta;

            //Per object
            alignas(16) glm::mat4 modelMatrix;
        };
        static ConstantBufferData constantBufferData;
        static Ref<Buffer> constantBuffer;
        static Ref<DescriptorSet> descriptorSet;
        static Ref<DescriptorSetLayout> descriptorSetLayout;

        static GraphicsContext *graphicsContext;
    };
}