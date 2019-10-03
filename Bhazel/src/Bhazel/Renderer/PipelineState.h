#pragma once

#include "Bhazel/Renderer/Buffer.h"


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

    struct Viewport {
        float left = 0.0f;
        float top = 0.0f;
        float width = 800.0f;
        float height = 600.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
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

    enum StencilOperation {
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

    enum ColorMaskFlags {
        Disable = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = (((Red | Green) | Blue) | Alpha)
    };

    struct BlendingStateAttachment {
        bool enableBlending = false;
        BlendingFactor srcColorBlendingFactor = BlendingFactor::SourceAlpha;
        BlendingFactor dstColorBlendingFactor = BlendingFactor::OneMinusSourceAlpha;
        BlendingOperation colorBlendingOperation = BlendingOperation::Add;
        BlendingFactor srcAlphaBlendingFactor = BlendingFactor::One;
        BlendingFactor dstAlphaBlendingFactor = BlendingFactor::One;
        BlendingOperation alphaBlendingOperation = BlendingOperation::Add;
        uint32 writeMask = ColorMaskFlags::All;
    };

    struct BlendingState
    {
        //TODO: logic operations
        //bool enableLogicOperations = false;
        //LogicOperation logicOperation; 
        std::vector<BlendingStateAttachment> attachmentBlendingStates;
        glm::vec4 blendingConstants = {};
    };

    class Shader;
    class Framebuffer;

    struct PipelineStateData {
        Ref<Shader> shader;
        DataLayout dataLayout; //Supporting a single vertex buffer
        //std::vector<Ref<Buffer>> indexBuffers; //TODO
        PrimitiveTopology primitiveTopology;
        std::vector<Viewport> viewports;
        RasterizerState rasterizerState;
        MultisampleState multiSampleState;
        DepthStencilState depthStencilState;
        BlendingState blendingState;
        Ref<Framebuffer> framebuffer; //Used to get the RenderPass (on Vulkan)
    };

    class PipelineState {
    public:
        static Ref<PipelineState> create(PipelineStateData& data);

        const PipelineStateData &getData() const { return data; }

    protected:
        explicit PipelineState(const PipelineStateData &data);
        virtual ~PipelineState() = default;

    private:
        const PipelineStateData data;
    };
}
