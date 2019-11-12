#pragma once

#include "Graphics/Buffer.h"


namespace BZ {

    enum class TestFunction {
        Always, Never,
        Less, LessOrEqual,
        Greater, GreaterOrEqual,
        Equal, NotEqual
    };

    enum class PrimitiveTopology {
        Points,
        Lines,
        Triangles,
        LineStrip,
        TriangleStrip
    };

    template<typename T>
    struct Rect {
        T left, top, width, height;
    };

    struct Viewport {
        Rect<float> rect = { 0, 0, 800, 600 };
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };

    struct ScissorRect {
        Rect<uint32> rect = { 0 };
    };

    enum class PolygonMode {
        Fill,
        Line,
        Point
    };

    enum class CullMode {
        None,
        Front,
        Back,
        FrontAndBack
    };

    struct RasterizerState {
        bool enableDepthClamp = false;
        bool enableRasterizerDiscard = false;
        PolygonMode polygonMode = PolygonMode::Fill;
        CullMode cullMode = CullMode::None;
        bool frontCounterClockwise = false;
        bool enableDepthBias = false;
        float depthBiasConstantFactor = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
        float lineWidth = 1.0f;
    };

    struct MultisampleState {
        uint32 sampleCount = 1; //Multiple of two
        bool enableSampleShading = false;
        float minSampleShading = 0.0f;
        //TODO missing sample mask
        bool enableAlphaToCoverage = false;
        bool enableAlphaToOne = false;
    };

    enum class StencilOperation {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        IncrementAndWrap,
        DecrementAndWrap,
        Invert
    };

    struct StencilOperationState {
        StencilOperation failOp = StencilOperation::Keep;
        StencilOperation depthFailOp = StencilOperation::Keep;
        StencilOperation passOp = StencilOperation::Keep;
        TestFunction testFunction = TestFunction::Never;
    };

    struct DepthStencilState {
        bool enableDepthTest = false;
        bool enableDepthWrite = false;
        TestFunction depthTestFunction = TestFunction::Never;
        bool enableDepthBoundsTest = false;
        bool enableStencilTest = false;
        StencilOperationState frontStencilOperation;
        StencilOperationState backStencilOperation;
        float minDepthBounds = 0.0f;
        float maxDepthBounds = 0.0f;
    };

    enum class BlendingFactor {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        AlphaSaturate, //(R, G, B) = (f, f, f) with f = min(As, 1 - Ad). A = 1

        //Dual Source Blending (Fragment shader outputting 2 colors to the same buffer)
        Source1Color,
        OneMinusSource1Color,
        Source1Alpha,
        OneMinusSource1Alpha
    };

    enum class BlendingOperation {
        Add,
        SourceMinusDestination,
        DestinationMinusSource,

        //Ignores BlendingFunction
        Min,
        Max
    };

    enum class ColorMaskFlags {
        Disable = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = Red | Green | Blue | Alpha
    };

    EnumClassFlagOperators(ColorMaskFlags);

    struct BlendingStateAttachment {
        bool enableBlending = false;
        BlendingFactor srcColorBlendingFactor = BlendingFactor::SourceAlpha;
        BlendingFactor dstColorBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        BlendingOperation colorBlendingOperation = BlendingOperation::Add;
        BlendingFactor srcAlphaBlendingFactor = BlendingFactor::One;
        BlendingFactor dstAlphaBlendingFactor = BlendingFactor::One;
        BlendingOperation alphaBlendingOperation = BlendingOperation::Add;
        uint8 writeMask = flagsToMask(ColorMaskFlags::All);
    };

    struct BlendingState
    {
        //TODO: logic operations
        //bool enableLogicOperations = false;
        //LogicOperation logicOperation; 
        std::vector<BlendingStateAttachment> attachmentBlendingStates;
        glm::vec4 blendingConstants = {};
    };

    enum class DynamicState {
        Viewport,
        Scissor,
        LineWidth,
        DepthBias,
        BlendConstants,
        DepthBounds,
        StencilCompareMask,
        StencilWriteMask,
        StencilReference
    };

    class Shader;
    class Framebuffer;
    class DescriptorSetLayout;

    struct PipelineStateData {

        //Supporting a single vertex buffer.
        DataLayout dataLayout;

        Ref<Shader> shader;
        PrimitiveTopology primitiveTopology;
        std::vector<Ref<DescriptorSetLayout>> descriptorSetLayouts;
        std::vector<Viewport> viewports;
        std::vector<ScissorRect> scissorRects;
        RasterizerState rasterizerState;
        MultisampleState multiSampleState;
        DepthStencilState depthStencilState;
        BlendingState blendingState;
        std::vector<DynamicState> dynamicStates;

        //Used on Vulkan only to get the RenderPass. 
        //If absent, it's assumed to be a swapchain framebuffer (the current frame one), which is fine becaues all of them have a similar RenderPass.
        Ref<Framebuffer> framebuffer;
    };

    class PipelineState {
    public:
        static Ref<PipelineState> create(PipelineStateData& data);

        const PipelineStateData& getData() const { return data; }

    protected:
        explicit PipelineState(PipelineStateData &inData);
        virtual ~PipelineState() = default;

    protected:
        PipelineStateData data;
    };
}