#pragma once

#include "Graphics/GpuObject.h"
#include "Graphics/Internal/VulkanIncludes.h"

#include "Graphics/Buffer.h"


namespace BZ {

class Shader;
class RenderPass;
class DescriptorSetLayout;

class PipelineLayout : public GpuObject<VkPipelineLayout> {
  public:
    static Ref<PipelineLayout> create(const std::initializer_list<Ref<DescriptorSetLayout>> &descriptorSetLayouts,
                                      const std::initializer_list<VkPushConstantRange> &pushConstants = {});

    PipelineLayout(const std::initializer_list<Ref<DescriptorSetLayout>> &descriptorSetLayouts,
                   const std::initializer_list<VkPushConstantRange> &pushConstants = {});
    ~PipelineLayout();

    BZ_NON_COPYABLE(PipelineLayout);

  private:
    std::vector<Ref<DescriptorSetLayout>> descriptorSetLayouts;
};


/*-------------------------------------------------------------------------------------------*/
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
    // VkSampleMask sampleMask; TODO
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
    VkColorComponentFlags writeMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
};

struct BlendingState {
    // TODO: logic operations
    // bool enableLogicOperations = false;
    // LogicOperation logicOperation;
    std::vector<BlendingStateAttachment> attachmentBlendingStates;
    glm::vec4 blendingConstants = {};
};

struct PipelineStateData {

    PipelineStateData();

    // Supporting a single vertex buffer.
    DataLayout dataLayout;

    Ref<Shader> shader;
    VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    Ref<PipelineLayout> layout;
    std::vector<VkViewport> viewports;  // Defaults to Window Dimensions
    std::vector<VkRect2D> scissorRects; // Defaults to Window Dimensions
    RasterizerState rasterizerState;
    MultisampleState multiSampleState;
    DepthStencilState depthStencilState;
    BlendingState blendingState;
    std::vector<VkDynamicState> dynamicStates;
    Ref<RenderPass> renderPass;
    uint32 subPassIndex = 0;
};

class PipelineState : public GpuObject<VkPipeline> {
  public:
    static Ref<PipelineState> create(PipelineStateData &data);

    explicit PipelineState(PipelineStateData &inData);
    ~PipelineState();

    BZ_NON_COPYABLE(PipelineState);

    const PipelineStateData &getData() const { return data; }

    // Used with the FileWatcher for Shader hot-reloading.
    void reload();

  private:
    void init();
    void destroy();

    PipelineStateData data;
};
}
