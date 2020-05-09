#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

    class Buffer;
    class Framebuffer;
    class PipelineState;
    class DescriptorSet;
    union ClearValues;
    struct FrameStats;
    class Texture;
    
    class WindowResizedEvent;
    class GraphicsContext;

    /*
    * Low level Graphics API.
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

        //static void beginRenderPass(uint32 commandBufferId);
        static void beginRenderPass(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer);
        static void endRenderPass(uint32 commandBufferId);

        static void nextSubPass(uint32 commandBufferId);

        static void clearColorAttachments(uint32 commandBufferId, const VkClearColorValue &clearColor);
        static void clearColorAttachments(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const VkClearColorValue &clearColor);
        static void clearDepthStencilAttachment(uint32 commandBufferId, const VkClearDepthStencilValue &clearValue);
        static void clearDepthStencilAttachment(uint32 commandBufferId, const Ref<Framebuffer> &framebuffer, const VkClearDepthStencilValue &clearValue);

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
        static void setViewports(uint32 commandBufferId, uint32 firstIndex, const VkViewport viewports[], uint32 viewportCount);
        static void setScissorRects(uint32 commandBufferId, uint32 firstIndex, const VkRect2D rects[], uint32 rectCount);
        static void setDepthBias(uint32 commandBufferId, float constantFactor, float clamp, float slopeFactor);

        //Sync
        static void pipelineBarrierTexture(uint32 commandBufferId, const Ref<Texture> &texture);

        static void waitForDevice();

        static API api;

    private:
        friend class Application;
        static void onWindowResize(const WindowResizedEvent &ev);

        static void init();
        static void destroy();

        static void beginFrame();
        static void endFrame();

        static void onImGuiRender(const FrameStats &frameStats);
    };
}