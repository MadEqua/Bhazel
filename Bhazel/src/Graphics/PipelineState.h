#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"

#include "Graphics/Buffer.h"


namespace BZ {

    struct RasterizerState {
        bool enableDepthClamp = false;
        bool enableRasterizerDiscard = false;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        bool frontFaceCounterClockwise = true;
        bool enableDepthBias = false;
        float depthBiasConstantFactor = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
        float lineWidth = 1.0f;
    };

    struct MultisampleState {
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
        bool enableSampleShading = false;
        float minSampleShading = 0.0f;
        //VkSampleMask sampleMask; TODO
        bool enableAlphaToCoverage = false;
        bool enableAlphaToOne = false;
    };

    struct StencilOperationState {
        VkStencilOp failOp = VK_STENCIL_OP_KEEP;
        VkStencilOp depthFailOp = VK_STENCIL_OP_KEEP;
        VkStencilOp passOp = VK_STENCIL_OP_KEEP;
        VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
    };

    struct DepthStencilState {
        bool enableDepthTest = false;
        bool enableDepthWrite = false;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_NEVER;
        bool enableDepthBoundsTest = false;
        bool enableStencilTest = false;
        StencilOperationState frontStencilOperation;
        StencilOperationState backStencilOperation;
        float minDepthBounds = 0.0f;
        float maxDepthBounds = 0.0f;
    };

    struct BlendingStateAttachment {
        bool enableBlending = false;
        VkBlendFactor srcColorBlendingFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        VkBlendFactor dstColorBlendingFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        VkBlendOp colorBlendingOperation = VK_BLEND_OP_ADD;
        VkBlendFactor srcAlphaBlendingFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstAlphaBlendingFactor = VK_BLEND_FACTOR_ONE;
        VkBlendOp alphaBlendingOperation = VK_BLEND_OP_ADD;
        VkColorComponentFlags writeMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    };

    struct BlendingState {
        //TODO: logic operations
        //bool enableLogicOperations = false;
        //LogicOperation logicOperation; 
        std::vector<BlendingStateAttachment> attachmentBlendingStates;
        glm::vec4 blendingConstants = {};
    };

    class Shader;
    class RenderPass;
    class DescriptorSetLayout;

    struct PipelineStateData {

        //Supporting a single vertex buffer.
        DataLayout dataLayout;

        Ref<Shader> shader;
        VkPrimitiveTopology primitiveTopology;
        std::vector<Ref<DescriptorSetLayout>> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissorRects;
        RasterizerState rasterizerState;
        MultisampleState multiSampleState;
        DepthStencilState depthStencilState;
        BlendingState blendingState;
        std::vector<VkDynamicState> dynamicStates;
        Ref<RenderPass> renderPass;
        uint32 subPassIndex = 0;
    };

    struct PipelineStateHandles {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    class PipelineState : public GpuObject<PipelineStateHandles> {
    public:
        static Ref<PipelineState> create(PipelineStateData &data);

        explicit PipelineState(PipelineStateData &inData);
        ~PipelineState();

        const PipelineStateData& getData() const { return data; }

        //Used with the FileWatcher for Shader hot-reloading.
        void reload();

    private:
        void init();
        void destroy();

        PipelineStateData data;
    };
}
