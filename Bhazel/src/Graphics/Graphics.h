#pragma once

#include "Constants.h"

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Shader.h"


namespace BZ {

    class WindowResizedEvent;
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

        //Start recording getting a CommandBuffer from the CommandPool reserved to the current frame.
        static Ref<CommandBuffer> startRecording();
        static Ref<CommandBuffer> startRecording(const Ref<Framebuffer> &framebuffer);

        static void startScene(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix); //TODO: more frameData
        static void startObject(const Ref<CommandBuffer> &commandBuffer, const glm::mat4 &modelMatrix);

        static void bindVertexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset);
        static void bindIndexBuffer(const Ref<CommandBuffer> &commandBuffer, const Ref<Buffer> &buffer, uint32 offset);
        
        static void bindPipelineState(const Ref<CommandBuffer> &commandBuffer, const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSets(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet);
        static void bindDescriptorSet(const Ref<CommandBuffer> &commandBuffer, const Ref<DescriptorSet> &descriptorSet, 
                                      const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                      uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

        static void draw(const Ref<CommandBuffer> &commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        static void drawIndexed(const Ref<CommandBuffer> &commandBuffer, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        //Pipeline dynamic state changes
        static void setViewports(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount);
        static void setScissorRects(const Ref<CommandBuffer> &commandBuffer, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount);

        static void endObject();
        static void endScene();

        static void endRecording(const Ref<CommandBuffer> &commandBuffer);

        static void submitCommandBuffer(const Ref<CommandBuffer> &commandBuffer);

        static void waitForDevice();

        static Ref<DescriptorSetLayout> getDescriptorSetLayout() { return descriptorSetLayout; }

        static API api;

    private:
        friend class Application;
        static void onWindowResize(WindowResizedEvent &ev);

        static void init();
        static void destroy();

        static void startFrame();
        static void endFrame();

        struct alignas(256) FrameConstantBufferData { //TODO: check this align value
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            alignas(16) glm::vec3 cameraPosition;
            alignas(8) glm::vec2 timeAndDelta;
        };

        struct alignas(256) ObjectConstantBufferData { //TODO: check this align value
            glm::mat4 modelMatrix;
        };


        static Ref<Buffer> frameConstantBuffer;
        static Ref<Buffer> objectConstantBuffer;

        static BufferPtr frameConstantBufferPtr;
        static BufferPtr objectConstantBufferPtr;

        static Ref<DescriptorSet> frameDescriptorSet;
        static Ref<DescriptorSet> objectDescriptorSet;

        //Same layout for both DescriptorSets.
        static Ref<DescriptorSetLayout> descriptorSetLayout;

        //Dummy data, except for the DescriptorSetLayout list, which contains the above descriptorSetLayout.
        //Used to bind engine DescriptorSets at any code place.
        static Ref<PipelineState> dummyPipelineState;

        static GraphicsContext *graphicsContext;

        static uint32 currentObjectIndex;
        static bool shouldBindFrameDescriptorSet;
    };
}