#pragma once

#include "Constants.h"

#include "Graphics/Buffer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Shader.h"
#include "Graphics/Color.h"
#include "Graphics/CommandBuffer.h"


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
        static uint32 beginCommandBuffer();
        static void endCommandBuffer(uint32 commandBufferId);

        static void clearColorAttachments(uint32 commandBufferId, const ClearValues &clearColor);
        static void clearColorAttachments(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearColor);
        static void clearDepthStencilAttachment(uint32 commandBufferId, const ClearValues &clearValue);
        static void clearDepthStencilAttachment(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const ClearValues &clearValue);

        static void beginScene(uint32 commandBufferId, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix); //TODO: more frameData
        static void beginObject(uint32 commandBufferId, const glm::mat4 &modelMatrix);

        //Vertex or index buffer
        static void bindBuffer(uint32 commandBufferId, const Ref<Buffer> &buffer, uint32 offset);
        
        static void bindPipelineState(uint32 commandBufferId, const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSets(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet);
        static void bindDescriptorSet(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet, 
                                      const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                      uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

        static void draw(uint32 commandBufferId, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        static void drawIndexed(uint32 commandBufferId, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        //Pipeline dynamic state changes
        static void setViewports(uint32 commandBufferId, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount);
        static void setScissorRects(uint32 commandBufferId, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount);

        static void waitForDevice();

        static Ref<DescriptorSetLayout> getDescriptorSetLayout() { return descriptorSetLayout; }

        static API api;

    private:
        friend class Application;
        static void onWindowResize(WindowResizedEvent &ev);

        static void init();
        static void destroy();

        static void beginFrame();
        static void endFrame();

        static Ref<CommandBuffer> commandBuffers[MAX_COMMAND_BUFFERS];
        static uint32 nextCommandBufferIndex;

        struct alignas(256) FrameConstantBufferData { //TODO: check this align value
            glm::vec2 timeAndDelta;
        };

        struct alignas(256) SceneConstantBufferData { //TODO: check this align value
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;
            alignas(16) glm::vec3 cameraPosition;
        };

        struct alignas(256) ObjectConstantBufferData { //TODO: check this align value
            glm::mat4 modelMatrix;
        };

        constexpr static uint32 MAX_SCENES_PER_FRAME = 16;
        constexpr static uint32 MAX_OBJECTS_PER_FRAME = 512;

        constexpr static uint32 FRAME_CONSTANT_BUFFER_SIZE = sizeof(FrameConstantBufferData);
        constexpr static uint32 SCENE_CONSTANT_BUFFER_SIZE = sizeof(SceneConstantBufferData) * MAX_SCENES_PER_FRAME;
        constexpr static uint32 OBJECT_CONSTANT_BUFFER_SIZE = sizeof(ObjectConstantBufferData) * MAX_OBJECTS_PER_FRAME;

        constexpr static uint32 FRAME_CONSTANT_BUFFER_OFFSET = 0;
        constexpr static uint32 SCENE_CONSTANT_BUFFER_OFFSET = FRAME_CONSTANT_BUFFER_SIZE;
        constexpr static uint32 OBJECT_CONSTANT_BUFFER_OFFSET = FRAME_CONSTANT_BUFFER_SIZE + SCENE_CONSTANT_BUFFER_SIZE;

        static Ref<Buffer> constantBuffer;

        static BufferPtr frameConstantBufferPtr;
        static BufferPtr sceneConstantBufferPtr;
        static BufferPtr objectConstantBufferPtr;

        static Ref<DescriptorSet> frameDescriptorSet;
        static Ref<DescriptorSet> sceneDescriptorSet;
        static Ref<DescriptorSet> objectDescriptorSet;

        //Same layout for all DescriptorSets.
        static Ref<DescriptorSetLayout> descriptorSetLayout;

        //Dummy data, except for the DescriptorSetLayout list, which contains the above descriptorSetLayout.
        //Used to bind engine DescriptorSets at any code place.
        static Ref<PipelineState> dummyPipelineState;

        static GraphicsContext *graphicsContext;

        static uint32 currentSceneIndex;
        static uint32 currentObjectIndex;
    };
}