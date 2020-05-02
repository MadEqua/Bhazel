#pragma once

#include "Graphics/Color.h"
#include "Graphics/PipelineState.h"


namespace BZ {

    class Framebuffer;
    class Buffer;
    class PipelineState;
    class DescriptorSet;
    class Texture;

    enum class CommandType {
        BeginRenderPass,
        EndRenderPass,
        ClearColorAttachments,
        ClearDepthStencilAttachment,
        BindBuffer,             //Vertex and Index
        BindPipelineState,
        BindDescriptorSet,
        SetPushConstants,
        Draw,
        DrawIndexed,            //Requires Dynamic State
        SetViewports,           //Requires Dynamic State
        SetScissorRects,        //Requires Dynamic State
        SetDepthBias,           //Requires Dynamic State
        PipelineBarrierTexture
    };

    struct Command {
        //Redefine this here because we don't want the default initialized ones that are defined on PipelineState.
        //We want a trivial default constructor for Command, and that means no default inits.
        template<typename T>
        struct Rect {
            T left, top, width, height;
        };

        struct Viewport {
            Rect<float> rect;
            float minDepth;
            float maxDepth;
        };

        struct ScissorRect {
            Rect<uint32> rect;
        };

        CommandType type;

        union {
            struct BeginRenderPassData {
                Framebuffer *framebuffer;
                bool forceClearAttachments;
            } beginRenderPassData;

            struct ClearAttachmentsData {
                Framebuffer *framebuffer;
                ClearValues clearValue;
            } clearAttachmentsData;

            struct BindBufferData {
                Buffer *buffer;
                uint32 offset;
            } bindBufferData;

            struct BindPipelineStateData {
                PipelineState *pipelineState;
            } bindPipelineStateData;

            struct BindDescriptorSetData {
                DescriptorSet *descriptorSet;
                PipelineState *pipelineState;
                uint32 setIndex;
                uint32 dynamicBufferOffsets[MAX_DESCRIPTOR_DYNAMIC_OFFSETS];
                uint32 dynamicBufferCount;
            } bindDescriptorSetData;

            struct SetPushConstants {
                PipelineState* pipelineState;
                uint8 shaderStageMask;
                byte data[MAX_PUSH_CONSTANT_SIZE];
                uint32 size;
                uint32 offset;
            } setPushConstantsData;

            struct DrawData {
                uint32 vertexCount;
                uint32 instanceCount;
                uint32 firstVertex;
                uint32 firstInstance;
            } drawData;

            struct DrawIndexedData {
                uint32 indexCount;
                uint32 instanceCount;
                uint32 firstIndex;
                uint32 vertexOffset;
                uint32 firstInstance;
            } drawIndexedData;

            struct SetViewportsData {
                uint32 firstIndex;
                Viewport viewports[MAX_VIEWPORTS];
                uint32 viewportCount;
            } setViewportsData;

            struct SetScissorRectsData {
                uint32 firstIndex;
                ScissorRect rects[MAX_VIEWPORTS];
                uint32 rectCount;
            } setScissorRectsData;

            struct SetDepthBiasData {
                float constantFactor;
                float clamp;
                float slopeFactor;
            } setDepthBiasData;

            struct PipelineBarrierTextureData {
                Texture *texture;
            } pipelineBarrierTexture;
        };
    };


    /*
    * CommmandBuffers contain a set of Commands that will be optimized (reordered, remove redundancies, etc) before creating the actual graphics API commands.
    * CommandPools create the CommandBuffers.
    */
    class CommandBuffer {
    public:
        Command& addCommand(CommandType type);

        void resetIndex() { nextCommandIndex = 0; }

        uint32 getCommandCount() const { return nextCommandIndex; }

        void optimizeAndGenerate();

    protected:
        Command commands[MAX_COMMANDS_PER_BUFFER];
        uint32 nextCommandIndex = 0;

        virtual void begin() = 0;
        virtual void end() = 0;

        /////////////////////////////////////////////////////////
        //Real command generation functions
        /////////////////////////////////////////////////////////
        virtual void beginRenderPass(const Framebuffer &framebuffer, bool forceClearAttachments) = 0;
        virtual void endRenderPass() = 0;

        virtual void clearColorAttachments(const Framebuffer &framebuffer, const ClearValues &clearColor) = 0;
        virtual void clearDepthStencilAttachment(const Framebuffer&framebuffer, const ClearValues &clearValue) = 0;

        virtual void bindBuffer(const Buffer &buffer, uint32 offset) = 0;

        virtual void bindPipelineState(const PipelineState &pipelineState) = 0;
        //virtual void bindDescriptorSets(const DescriptorSet&descriptorSet) = 0;
        virtual void bindDescriptorSet(const DescriptorSet &descriptorSet,
                                       const PipelineState &pipelineState, uint32 setIndex,
                                       uint32 dynamicBufferOffsets[], uint32 dynamicBufferCount) = 0;

        virtual void setPushConstants(const PipelineState &pipelineState, uint8 shaderStageMask,
                                      const void* data, uint32 size, uint32 offset) = 0;

        virtual void draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
        virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;

        //Pipeline dynamic state changes
        virtual void setViewports(uint32 firstIndex, const Command::Viewport viewports[], uint32 viewportCount) = 0;
        virtual void setScissorRects(uint32 firstIndex, const Command::ScissorRect rects[], uint32 rectCount) = 0;
        virtual void setDepthBias(float constantFactor, float clamp, float slopeFactor) = 0;

        //Sync
        virtual void pipelineBarrierTexture(const Texture &texture) = 0;
    };
}
