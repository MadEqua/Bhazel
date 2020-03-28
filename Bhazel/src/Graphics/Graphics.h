#pragma once

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
    class Scene;

    /*
    * Low level Graphics API.
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

        static void beginScene(uint32 commandBufferId, const Ref<PipelineState>& pipelineState, const glm::vec3 &cameraPosition, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
        static void beginScene(uint32 commandBufferId, const Ref<PipelineState> &pipelineState, const Scene &scene);
        static void beginObject(uint32 commandBufferId, const Ref<PipelineState> &pipelineState, const glm::mat4 &modelMatrix, const glm::mat3 &normalMatrix);

        //Vertex or index buffer
        static void bindBuffer(uint32 commandBufferId, const Ref<Buffer> &buffer, uint32 offset);
        
        static void bindPipelineState(uint32 commandBufferId, const Ref<PipelineState> &pipelineState);
        //static void bindDescriptorSets(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet);
        static void bindDescriptorSet(uint32 commandBufferId, const Ref<DescriptorSet> &descriptorSet, 
                                      const Ref<PipelineState> &pipelineState, uint32 setIndex,
                                      uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount);

        static void setPushConstants(uint32 commandBufferId, const Ref<PipelineState> &pipelineState, uint8 shaderStageMask,
                                     const void *data, uint32 size, uint32 offset);

        static void draw(uint32 commandBufferId, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
        static void drawIndexed(uint32 commandBufferId, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);

        //Pipeline dynamic state changes
        static void setViewports(uint32 commandBufferId, uint32 firstIndex, const Viewport viewports[], uint32 viewportCount);
        static void setScissorRects(uint32 commandBufferId, uint32 firstIndex, const ScissorRect rects[], uint32 rectCount);

        static void waitForDevice();

        static Ref<DescriptorSetLayout>& getDefaultDescriptorSetLayout();

        static API api;

    private:
        friend class Application;
        static void onWindowResize(const WindowResizedEvent &ev);

        static void init();
        static void destroy();

        static void beginFrame();
        static void endFrame();
    };
}